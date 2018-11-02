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

int main(void)
{
  struct sockaddr_in si_me, si_other;
  socklen_t slen=sizeof(si_other);
  int s, i;
  char buf[BUFLEN];

 //  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
 //  {
	// diep("socket");
 //  }

  SocketResult r = SocketNew(DOMAIN_IPV4, &s);
  if (r != SOCKETRESULT_OK) {
      return 1;
  }

  Address address = { DOMAIN_IPV4, {0,0,0,0x0100007f} };

  r = SocketBind(s, &address, PORT);
  if (r != SOCKETRESULT_OK) {
      return 1;
  }

  // memset((char *) &si_me, 0, sizeof(si_me));
  // si_me.sin_family = AF_INET;
  // si_me.sin_port = htons(PORT);
  // si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  // if (bind(s, (struct sockaddr*)&si_me, sizeof(si_me))==-1)
  //     diep("bind");

  printf("\nServer started...\n");

  for (i=0; i<NPACK; i++) {
    //if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen)==-1) {

    Address client_address = { 0 };
    uint16_t client_port = 0;
    if (SocketReceiveFrom(s, buf, BUFLEN, 0, &client_address, &client_port) != SOCKETRESULT_OK) {
      diep("recvfrom()");
    }
    printf("Received packet from %08x:%d\nData: %s\n\n",
            client_address.address[3], ntohs(client_port), buf);
           //inet_ntoa(client_address.address[3]), ntohs(client_port), buf);
  }

  close(s);
  return 0;
}
