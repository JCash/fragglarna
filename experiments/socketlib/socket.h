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
} Address;



// Creates an udp socket
SocketResult SocketNew(Domain domain, Socket* socket);

// Destroys a previously created socket
SocketResult SocketDelete(Socket socket);

// Binds a socket to an address and a port (for the server)
SocketResult SocketBind(Socket s, Address* address, int port);

SocketResult SocketSend(Socket socket, const void* buffer, int length, int* sent_bytes);

// From client to server
SocketResult SocketSendTo(Socket socket, const void* buffer, int length, int* sent_bytes, Address* to_addr, uint16_t to_port);

SocketResult SocketReceive(Socket socket, const void* buffer, int length, int* received_bytes);

SocketResult SocketReceiveFrom(Socket socket, const void* buffer, int length, int* received_bytes, Address* from_addr, uint16_t* from_port);
