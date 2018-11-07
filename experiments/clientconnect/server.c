#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h> // exit
#include <string.h>	// memset
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"
#include "socket.h"

#define BUFLEN 512
#define NPACK 10

void diep(char *s)
{
  perror(s);
  exit(1);
}

int main(int argc, const char** argv)
{
    int s;

    SocketResult r = SocketNew(FAMILY_IPV4, PROTOCOL_UDP, &s);
    if (r != SOCKETRESULT_OK) {
        fprintf(stderr, "Failed to create socket: %s", SockerErrorString());
        return 1;
    }

    r = SocketSetSockoptBool(s, SOL_SOCKET, SO_BROADCAST, true);
    if (r != SOCKETRESULT_OK) {
        fprintf(stderr, "Failed to set SO_BROADCAST to socket: %s", SockerErrorString());
        return 1;
    }

    r = SocketBind(s, 0, PORT);
    if (r != SOCKETRESULT_OK) {
        fprintf(stderr, "Failed to set socket to port %d: %s", PORT, SockerErrorString());
        return 1;
    }

    r = SocketSetBlocking(s, false);
    if (r != SOCKETRESULT_OK) {
        fprintf(stderr, "Failed to set socket to non blocking: %s", SockerErrorString());
        return 1;
    }

    printf("\nServer started...\n");

    uint32_t sleepms = 1000 / 60;

    const char* dial = "|/-\\";
    uint32_t tick = 0;
    while (true)
    {
        r = SocketSleep(s, sleepms);
        if (r == SOCKETRESULT_OK)
        {
            Address client_address = { 0 };
            uint16_t client_port = 0;
            char buf[BUFLEN];
            if (SocketReceiveFrom(s, buf, BUFLEN, 0, &client_address, &client_port) != SOCKETRESULT_OK) {
                diep("recvfrom()");
            }

            char* addr = AddressToIPString(&client_address);

            printf("Received packet from %s:%d\nData: %s\n", addr, ntohs(client_port), buf);

            free(addr);
        }
        else if(r == SOCKETRESULT_FAIL)
        {
            printf("Error, aborting\n");
            break;
        }
        printf("\b%c", dial[(tick/12)%4]);
        fflush(stdout);
        ++tick;
    }

  // for (int i=0; i<NPACK; i++) {
  //   Address client_address = { 0 };
  //   uint16_t client_port = 0;
  //   if (SocketReceiveFrom(s, buf, BUFLEN, 0, &client_address, &client_port) != SOCKETRESULT_OK) {
  //     diep("recvfrom()");
  //   }

  //   char* addr = AddressToIPString(&client_address);

  //   printf("Received packet from %s:%d\nData: %s\n\n", addr, ntohs(client_port), buf);

  //   free(addr);
  // }

    close(s);
    printf("Exiting server\n");
    return 0;
}
