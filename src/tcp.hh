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

#include "thread.hh"

class TCPServer : public ThreadedClass {
public:

	static void initSocketAddress(struct sockaddr_in *name,
																const char *hostname,
																uint16_t port);
	static int makeSocket(struct sockaddr_in *name, uint16_t port);
	
 protected:
  uint16_t port;
  int fd;

 public:
  TCPServer(const char *hostname, uint16_t port);

  virtual ~TCPServer();

  virtual void start();
  virtual void stop();

 protected:
  virtual void doWork();
};

class TCPClient : public ThreadedClass {
 protected:
  int fd;

 public:
  TCPClient(int _fd);

  virtual ~TCPClient();

 protected:
  virtual void doWork();
};


#endif /* TCP_H__ */
