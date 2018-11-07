
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

    for (int i=0; i<NPACK; i++)
    {
        printf("Sending packet %d\n", i);
        char buf[BUFLEN];
        sprintf(buf, "This is packet %d\n", i);
        if (SocketSendTo(s, buf, BUFLEN, 0, &serverAddress, PORT) != SOCKETRESULT_OK)
        {
          diep("sendto()");
        }
    }

    close(s);
    return 0;
}
