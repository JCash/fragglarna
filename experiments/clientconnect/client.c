
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h> // exit
#include <string.h> // memset
#include <unistd.h>
#include <netdb.h>

#include "common.h"
#include "socket.h"

#define BUFLEN 512
#define NPACK 10

#include <arpa/inet.h>
#include <netinet/in.h>

void diep(char *s)
{
  perror(s);
  exit(1);
}

void Usage()
{
    printf("Usage: client <serverAddress>\n");
}

int main(int argc, const char** argv)
{
    Address serverAddress;

    const char* ipaddress = SRV_IP;
    if (argc > 1)
        ipaddress = argv[1];
    SocketResult r = AddressFromIPString(ipaddress, &serverAddress);
    if (r != SOCKETRESULT_OK) {
        Usage();
        return 1;
    }

    char* addr = AddressToIPString(&serverAddress);
    printf("Sending packet to '%s'\n", addr);
    free(addr);

    int s;
    r = SocketNew(FAMILY_IPV4, PROTOCOL_UDP, &s);
    if (r != SOCKETRESULT_OK) {
        Usage();
        return 1;
    }

    r = SocketSetSockoptBool(s, SOL_SOCKET, SO_BROADCAST, true);
    if (r != SOCKETRESULT_OK) {
        Usage();
        return 1;
    }

    r = SocketSetBlocking(s, false);
    if (r != SOCKETRESULT_OK) {
        Usage();
        return 1;
    }

    char buf[BUFLEN];
    *((uint32_t*)buf) = 0xFFFFFFFF;
    const char* msg_connect = "connect";
    sprintf(buf+4, "%s", msg_connect);

    if (SocketSendTo(s, buf, strlen(buf), 0, &serverAddress, PORT) != SOCKETRESULT_OK)
    {
      diep("sendto('connect')");
    }

    // for (int i=0; i<NPACK; i++)
    // {
    //     printf("Sending packet %d\n", i);
    //     char buf[BUFLEN];
    //     *((uint32_t*)buf) = 0xFFFFFFFF;
    //     sprintf(buf+4, "connect\nThis is packet %d", i);
    //     if (SocketSendTo(s, buf, BUFLEN, 0, &serverAddress, PORT) != SOCKETRESULT_OK)
    //     {
    //       diep("sendto()");
    //     }
    // }

    while (1)
    {
        int received_bytes = 0;
        if (SocketReceive(s, buf, sizeof(buf), &received_bytes) == SOCKETRESULT_OK) {
            buf[sizeof(buf)-1] = 0;

            printf("%s\n", buf);
        }
    }

    close(s);
    return 0;
}
