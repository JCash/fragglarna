#include "socket.h"
#include "common.h"

#include <assert.h>
#include <stdio.h>		// printf
#include <string.h>		// memset
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#if defined(__APPLE__)
#include <unistd.h>		// close
#endif

static int DomainToAF(Domain domain)
{
	int ipdomain = AF_INET6;
	if (domain == DOMAIN_IPV4)
		ipdomain = AF_INET;
	return ipdomain;
}


static int IsSocketIPv4(Socket socket)
{
#if defined(_WIN32)
    WSAPROTOCOL_INFO ss = {0};
    socklen_t sslen = sizeof(ss);
    int result = getsockopt( socket, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&ss, (int*)&sslen );
    if (result == 0) {
        return ss.iAddressFamily == AF_INET;
    }
#else
    struct sockaddr_storage ss = { 0 };
    socklen_t sslen = sizeof(ss);
    int result = getsockname(socket, (struct sockaddr*) &ss, &sslen);
    if (result == 0)
    {
        return ss.ss_family == AF_INET;
    }
#endif
    printf("Failed to retrieve address family\n");

    return 0;
}

static int IsSocketIPv6(Socket socket)
{
#if defined(_WIN32)
    WSAPROTOCOL_INFO ss = {0};
    socklen_t sslen = sizeof(ss);
    int result = getsockopt( socket, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&ss, (int*)&sslen );
    if (result == 0) {
        return ss.iAddressFamily == AF_INET6;
    }
#else
    struct sockaddr_storage ss = { 0 };
    socklen_t sslen = sizeof(ss);
    int result = getsockname(socket, (struct sockaddr*) &ss, &sslen);
    if (result == 0)
    {
        return ss.ss_family == AF_INET6;
    }
#endif

    printf("Failed to retrieve address domain\n");
    return 0;
}

static inline uint32_t* IPv4(Address* address)
{
    assert(address->domain == DOMAIN_IPV4);
    return &address->address[3];
}

static inline uint32_t* IPv6(Address* address)
{
    assert(address->domain == DOMAIN_IPV6);
    return &address->address[0];
}

SocketResult SocketNew(Domain domain, Socket* s)
{
	int ipdomain = DomainToAF(domain);
  	if ((*s = socket(ipdomain, SOCK_DGRAM, IPPROTO_UDP)) == -1 )
  	{
  		printf("Failed to open socket");
  		return SOCKETRESULT_FAIL;
  	}

    return SOCKETRESULT_OK;
}

SocketResult SocketDelete(Socket socket)
{
	close(socket);
    return SOCKETRESULT_OK;
}


SocketResult SocketBind(Socket socket, Address* address, int port)
{
    int result = 1;
    if (IsSocketIPv4(socket))
    {
	    struct sockaddr_in sock_addr = { 0 };
	    sock_addr.sin_family = AF_INET;
	    sock_addr.sin_addr.s_addr = *IPv4(address);
	    sock_addr.sin_port = htons(port);
	    result = bind(socket, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
	}
    return result == 0 ? SOCKETRESULT_OK : SOCKETRESULT_FAIL;
}

SocketResult SocketSend(Socket socket, const void* buffer, int length, int* sent_bytes)
{
	if (sent_bytes)
    	*sent_bytes = 0;
#if defined(__linux__)
    ssize_t s = send(socket, buffer, length, MSG_NOSIGNAL);
#elif defined(_WIN32)
    int s = send(socket, (const char*) buffer, length, 0);
#else
    ssize_t s = send(socket, buffer, length, 0);
#endif
    printf("%s:%d: result %zd\n", __FUNCTION__, __LINE__, s);
    if (s < 0)
    {
        //return NativeToResultCompat(DM_SOCKET_ERRNO);
        return SOCKETRESULT_FAIL;
    }
    else
    {
    	if (sent_bytes)
        	*sent_bytes = s;
        return SOCKETRESULT_OK;
    }
}

#include "common.h"
SocketResult SocketSendTo(Socket socket, const void* buffer, int length, int* sent_bytes, Address* to_addr, uint16_t to_port)
{
    struct sockaddr_in sock_addr = { 0 };
    sock_addr.sin_family = DomainToAF(to_addr->domain);
    //sock_addr.sin_addr.s_addr = *IPv4(to_addr);
    sock_addr.sin_port = htons(to_port);
    if (inet_aton(SRV_IP, &sock_addr.sin_addr)==0) {
            printf("inet_aton() failed\n");
            return SOCKETRESULT_FAIL;
        }

    int result = (int) sendto(socket, buffer, length, 0, (const struct sockaddr*) &sock_addr, sizeof(sock_addr));
    if (sent_bytes)
    	*sent_bytes = result >= 0 ? result : 0;
    return result >= 0 ? SOCKETRESULT_OK : SOCKETRESULT_FAIL;
}


SocketResult SocketReceive(Socket socket, const void* buffer, int length, int* received_bytes)
{
	int r = recv(socket, (void*)buffer, length, 0);
	if (r < 0)
		return SOCKETRESULT_FAIL;
	if (received_bytes)
		*received_bytes = r;
	return SOCKETRESULT_OK;
}

SocketResult SocketReceiveFrom(Socket socket, const void* buffer, int length, int* received_bytes, Address* from_addr, uint16_t* from_port)
{
    if (IsSocketIPv4(socket))
    {
        struct sockaddr_in sock_addr = { 0 };
        socklen_t addr_len = sizeof(sock_addr);
        int result = recvfrom(socket, (void*)buffer, length, 0, (struct sockaddr*) &sock_addr, &addr_len);
        if (result < 0)
        	return SOCKETRESULT_FAIL;

        if (from_addr)
        {
            from_addr->domain = DOMAIN_IPV4;
            *IPv4(from_addr) = sock_addr.sin_addr.s_addr;
        }
        if (from_port)
            *from_port = ntohs(sock_addr.sin_port);
        if (received_bytes)
            *received_bytes = result;

        return SOCKETRESULT_OK;
	}
	return SOCKETRESULT_FAIL;
}
