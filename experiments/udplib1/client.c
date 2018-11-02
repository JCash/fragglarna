
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h> // exit
#include <string.h> // memset
#include <unistd.h>

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

int main(void)
{
    int s, i;//, slen=sizeof(si_other);
    char buf[BUFLEN];

    SocketResult r = SocketNew(DOMAIN_IPV4, &s);
    if (r != SOCKETRESULT_OK) {
        return 1;
    }

    // if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    // diep("socket");

        // struct sockaddr_in si_other;
        // memset((char *) &si_other, 0, sizeof(si_other));
        // si_other.sin_family = AF_INET;
        // si_other.sin_port = htons(PORT);
        // if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
        //     fprintf(stderr, "inet_aton() failed\n");
        //     exit(1);
        // }
        // printf("Server IP: %s -> %x\n", SRV_IP, si_other.sin_addr.s_addr);

    Address si_other = { DOMAIN_IPV4, {0,0,0,0x0100007f} };

    for (i=0; i<NPACK; i++)
    {
        printf("Sending packet %d\n", i);
        sprintf(buf, "This is packet %d\n", i);
        //if (sendto(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, sizeof(si_other))==-1)
        if (SocketSendTo(s, buf, BUFLEN, 0, &si_other, PORT) != SOCKETRESULT_OK)
        //if (SocketSend(s, buf, BUFLEN, 0) != SOCKETRESULT_OK)
        {
          diep("sendto()");
        }
    }

    close(s);
    return 0;
}
