#include <list>
#include <string.h>

#include "avarice.h"

#include "tcp.hh"


/* server */
TCPServer::TCPServer(const char *_hostname, uint16_t _port) {
	strncpy(hostname, _hostname, sizeof(hostname));
	port = _port;
	fd = -1;
}

TCPServer::~TCPServer() {
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}
}

void TCPServer::start() {
	TCPServer::initSocketAddress(&name, hostname, port);
	fd = TCPServer::makeSocket(&name, port);
	console->statusOut("Waiting for connection on port %hu\n", port);
	SYSCALL_CHECK(listen(fd, 1));
	fds.add(fd);
	printf("server started\n");
	ThreadedClass::start();
}

void TCPServer::stop() {
	ThreadedClass::stop();
	if (fd != -1) {
		close(fd);
		fd = -1;
	}
}

void TCPServer::doWork() {
	while (!stopRequested) {
		fds.waitRead();
		if (fds.isSet(fd)) {
			struct sockaddr_in clientname;
			socklen_t size = (socklen_t)sizeof(clientname);
			int gfd = accept(fd, (struct sockaddr *)&clientname, &size);
			if (gfd < 0) {
				console->statusOut("Could not accept connection\n");
			} else {
				TCPClient *client = new TCPClient(gfd, &clientname);
				console->statusOut("Accepting connection and spawning client");
				client->start();
			}
		}
	}
}

/* client */
extern ThreadSafeList<LineIOClass *>listeners;

TCPClient::TCPClient(int _fd, struct sockaddr_in *_name) {
	fd = _fd;
	memcpy(&name, _name, sizeof(name));
	fds.add(fd);
	listeners.push_front(this); // XXX race condition
	const char *welcomeMessage = "Welcome to AvariceJS\n";
	write(fd, welcomeMessage, strlen(welcomeMessage));
}

TCPClient::~TCPClient() {
	if (fd >= 0)
		close(fd);
	listeners.remove(this); // XXX race condition
}

void TCPClient::doWork() {
	while (!stopRequested) {
		while (inputQueue.isDataAvailable()) {
			const string *str = inputQueue.getData();
			const char *ptr = str->c_str();
			int len = strlen(ptr);
			
			int ret = write(fd, ptr, len);
			
			delete str;
			
			if (ret <= 0) {
				console->errorOut("Error while writing %d bytes: %s\n", strerror(ret));
				delete this;
				return;
			} else if (ret != len) {
				console->errorOut("Could not write %d bytes, only %d\n", len, ret);
				delete this;
				return;
			}
		}

		fds.waitRead(10);
		if (fds.isSet(fd)) {
			// XXX concat data to big string and recognize \n
			char buf[256];
			int len = read(fd, buf, sizeof(buf));
			if (len <= 0) {
				console->errorOut("Error while reading:: %s\n", strerror(len));
				delete this;
				return;
			} else {
				buf[len] = 0;
				printf("read %s\n", buf);
			}
		}
	}
}

void TCPClient::print(const std::string &str) {
	inputQueue.putData(str);
}

bool TCPClient::isDataAvailable() {
	return outputQueue.isDataAvailable();
}

const string *TCPClient::getData() {
	return outputQueue.getData();
}
