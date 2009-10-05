#include "avarice.h"

#include <string.h>

#include "tcp.hh"

void TCPServer::initSocketAddress(struct sockaddr_in *name,
																	const char *hostname,
																	uint16_t port) {
	struct hostent *hostInfo;

	memset(name, 0, sizeof(*name));
	name->sin_family = AF_INET;
	name->sin_port = htons(port);
	// Try numeric interpretation (1.2.3.4) first, then
	// hostname resolution if that failed.
	if (inet_aton(hostname, &name->sin_addr) == 0) {
		hostInfo = gethostbyname(hostname);
		check(hostInfo != NULL, "Unknown host %s", hostname);
		name->sin_addr = *(struct in_addr *)hostInfo->h_addr;
	}
}

int TCPServer::makeSocket(struct sockaddr_in *name, uint16_t port)
{
	int sock;
	int tmp;
	struct protoent *protoent;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	unixCheck(sock);

	// Allow rapid reuse of this port.
	tmp = 1;
	unixCheck(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&tmp, sizeof(tmp)));

	// Enable TCP keep alive process.
	tmp = 1;
	unixCheck(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&tmp, sizeof(tmp)));

	unixCheck(bind(sock, (struct sockaddr *)name, sizeof(*name)));

	protoent = getprotobyname("tcp");
	check(protoent != NULL, "tcp protocol unknown (oops?)");

	tmp = 1;
	unixCheck(setsockopt(sock, protoent->p_proto, TCP_NODELAY,
											(char *)&tmp, sizeof(tmp)));

	return sock;
}

