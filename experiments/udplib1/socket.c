#include "socket.h"
#include "common.h"

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
		ipdomain = AF_INET6;
	return ipdomain;
}


static bool IsSocketIPv4(Socket socket)
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
    dmLogError("Failed to retrieve address family (%d): %s",
        NATIVETORESULT(DM_SOCKET_ERRNO), ResultToString(NATIVETORESULT(DM_SOCKET_ERRNO)));

    return false;
}

static bool IsSocketIPv6(Socket socket)
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

    dmLogError("Failed to retrieve address family (%d): %s",
        NATIVETORESULT(DM_SOCKET_ERRNO), ResultToString(NATIVETORESULT(DM_SOCKET_ERRNO)));
    return false;
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


SocketResult SocketBind(Socket s, Domain domain, const char* address, int port)
{
	struct sockaddr_in si_other;
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = DomainToAF(domain);
    si_other.sin_port = htons(port);
    if (inet_aton(address, &si_other.sin_addr)==0) {
  		printf("Failed to open socket");
  		return SOCKETRESULT_FAIL;
    }
    return SOCKETRESULT_OK;
}


SocketResult SocketSendTo(Socket socket, const void* buffer, int length, int* sent_bytes, Address to_addr, uint16_t to_port)
{
    struct sockaddr_in sock_addr = { 0 };
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = *IPv4(&to_addr);
    sock_addr.sin_port = htons(to_port);
}



