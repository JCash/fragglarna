
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h> // exit
#include <string.h> // memset
#include <unistd.h>

#include "common.h"

#define BUFLEN 512
#define NPACK 10



int main(void)
{
    int s, i;//, slen=sizeof(si_other);
    char buf[BUFLEN];

    int socket;
    SocketResult r = SocketNew(DOMAIN_IPV4, &socket);
    if (r != SOCKETRESULT_OK) {
        return 1;
    }

    // if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    // diep("socket");

    r = SocketBind(socket, DOMAIN_IPV4, SRV_IP, PORT);
    if (r != SOCKETRESULT_OK) {
        return 1;
    }

    // struct sockaddr_in si_other;
    // memset((char *) &si_other, 0, sizeof(si_other));
    // si_other.sin_family = AF_INET;
    // si_other.sin_port = htons(PORT);
    // if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
    // fprintf(stderr, "inet_aton() failed\n");
    // exit(1);
    // }

    for (i=0; i<NPACK; i++)
    {
        printf("Sending packet %d\n", i);
        sprintf(buf, "This is packet %d\n", i);
        //if (sendto(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, slen)==-1)
        if (SocketSendTo(socket, buf, BUFLEN, 0, (struct sockaddr*)&si_other, slen)==-1)
          diep("sendto()");
    }

    close(s);
    return 0;
}
