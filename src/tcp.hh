#ifndef TCP_H__
#define TCP_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <fcntl.h>

#include <vector>

#include "select.hh"
#include "thread.hh"
#include "terminal-io.hh"

class TCPClient : public ThreadedClass, public LineIOClass {
 protected:
  int fd;
	struct sockaddr_in name;
	FDSelect fds;

 public:
  ThreadSafeQueue<std::string> inputQueue;
  ThreadSafeQueue<std::string> outputQueue;

  TCPClient(int _fd, struct sockaddr_in *name);

  virtual ~TCPClient();

 protected:
  virtual void doWork();

public:
	void print(const std::string &str);
	bool isDataAvailable();
	const string *getData();
};

class TCPServer : public ThreadedClass {
public:

	static void initSocketAddress(struct sockaddr_in *name,
																const char *hostname,
																uint16_t port);
	static int makeSocket(struct sockaddr_in *name, uint16_t port);
	
 protected:
	char hostname[256];
  uint16_t port;

	static const int MAX_CLIENTS = 16;

  int fd;
	struct sockaddr_in name;

	FDSelect fds;

 public:
  TCPServer(const char *hostname, uint16_t port);

  virtual ~TCPServer();

  virtual void start();
  virtual void stop();

 protected:
  virtual void doWork();
};


#endif /* TCP_H__ */
