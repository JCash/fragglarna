#include "socket.h"
#include "common.h"

#include <assert.h>
#include <stdio.h>		// printf
#include <string.h>		// memset
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#if defined(__APPLE__)
#include <unistd.h>		// close
#endif

static int FamilyToAF(Family family)
{
	int af = AF_INET6;
	if (family == FAMILY_IPV4)
		af = AF_INET;
	return af;
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

    printf("Failed to retrieve address family\n");
    return 0;
}

static inline uint32_t* IPv4(Address* address)
{
    assert(address->family == FAMILY_IPV4);
    return &address->address[3];
}

static inline uint32_t* IPv6(Address* address)
{
    assert(address->family == FAMILY_IPV6);
    return &address->address[0];
}

SocketResult SocketNew(Family family, Protocol protocol, Socket* s)
{
	int af = FamilyToAF(family);
  	if ((*s = socket(af, SOCK_DGRAM, protocol == PROTOCOL_UDP ? IPPROTO_UDP : IPPROTO_TCP)) == -1 )
  	{
  		printf("Failed to open socket");
  		return SOCKETRESULT_FAIL;
  	}

#if defined(__MACH__)
    SocketSetSockoptBool(*s, SOL_SOCKET, SO_NOSIGPIPE, true); //use MSG_NOSIGNAL on linux
#endif

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
	    sock_addr.sin_addr.s_addr = address ? *IPv4(address) != 0 : htonl(INADDR_ANY);
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
    if (s < 0)
    {
        return SOCKETRESULT_FAIL;
    }
    else
    {
    	if (sent_bytes)
        	*sent_bytes = s;
        return SOCKETRESULT_OK;
    }
}

SocketResult SocketSendTo(Socket socket, const void* buffer, int length, int* sent_bytes, Address* to_addr, uint16_t to_port)
{
    struct sockaddr_in sock_addr = { 0 };
    sock_addr.sin_family = FamilyToAF(to_addr->family);
    sock_addr.sin_addr.s_addr = *IPv4(to_addr);
    sock_addr.sin_port = htons(to_port);

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
            from_addr->family = FAMILY_IPV4;
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

SocketResult SocketSetSockoptBool(Socket socket, int level, int name, bool flag)
{
    int b = (int)flag;
    int result = setsockopt(socket, level, name, (char*)&b, sizeof(b));
    return result >= 0 ? SOCKETRESULT_OK : SOCKETRESULT_FAIL;
}

char* AddressToIPString(Address* address)
{
    if (address->family == FAMILY_IPV4)
    {
        char addrstr[15 + 1] = { 0 };
        inet_ntop(AF_INET, IPv4(address), addrstr, sizeof(addrstr));
        return strdup(addrstr);
    }
    else if (address->family == FAMILY_IPV6)
    {
        char addrstr[45 + 1] = { 0 };
        inet_ntop(AF_INET6, IPv6(address), addrstr, sizeof(addrstr));
        return strdup(addrstr);
    }
    printf("Unsupported address family\n");
    return 0;
}

SocketResult GetHostByName(const char* name, Address* address)
{
    struct addrinfo hints;
    struct addrinfo* res;

    memset(&hints, 0x0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // AF_INET, AF_INET6
    hints.ai_socktype = SOCK_STREAM;

    SocketResult result = SOCKETRESULT_FAIL;
    if (getaddrinfo(name, NULL, &hints, &res) == 0)
    {
        memset(address, 0x0, sizeof(Address));
        int ipv4 = 1;
        int ipv6 = 1;
        struct addrinfo* iterator = res;
        while (iterator)
        {
            if (ipv4 && iterator->ai_family == AF_INET)
            {
                struct sockaddr_in* saddr = (struct sockaddr_in *) iterator->ai_addr;
                address->family = FAMILY_IPV4;
                *IPv4(address) = saddr->sin_addr.s_addr;
                result = SOCKETRESULT_OK;
                break;
            }
            else if (ipv6 && iterator->ai_family == AF_INET6)
            {
                struct sockaddr_in6* saddr = (struct sockaddr_in6 *) iterator->ai_addr;
                address->family = FAMILY_IPV6;
                memcpy(IPv6(address), &saddr->sin6_addr, sizeof(struct in6_addr));
                result = SOCKETRESULT_OK;
                break;
            }

            iterator = iterator->ai_next;
        }

        freeaddrinfo(res);
    }
    return result;
}

SocketResult AddressFromIPString(const char* hostname, Address* address)
{
    return GetHostByName(hostname, address);
}