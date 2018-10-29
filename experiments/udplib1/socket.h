#pragma once
#include <stdint.h>

typedef int Socket;

typedef enum Domain
{
    DOMAIN_IPV4,
    DOMAIN_IPV6,
} Domain;

typedef enum SocketResult
{
	SOCKETRESULT_OK,
	SOCKETRESULT_FAIL,
} SocketResult;

typedef struct Address
{
	Domain domain;
	unsigned int address[4]; // ipv4 starts at index 3
};



// Creates an udp socket
SocketResult SocketNew(Domain domain, Socket* socket);

// Destroys a previously created socket
SocketResult SocketDelete(Socket socket);

// Binds a socket to an address and a port
SocketResult SocketBind(Socket socket, Domain domain, const char* address, int port);

SocketResult SocketSend(Socket socket, const void* buffer, int length, int* sent_bytes);
SocketResult SocketSendTo(Socket socket, const void* buffer, int length, int* sent_bytes, Address* to_addr, uint16_t to_port);
