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
  char buf[BUFLEN];

  SocketResult r = SocketNew(FAMILY_IPV4, PROTOCOL_UDP, &s);
  if (r != SOCKETRESULT_OK) {
      return 1;
  }

  r = SocketSetSockoptBool(s, SOL_SOCKET, SO_BROADCAST, true);
  if (r != SOCKETRESULT_OK) {
      return 1;
  }

  r = SocketBind(s, 0, PORT);
  if (r != SOCKETRESULT_OK) {
      return 1;
  }

  printf("\nServer started...\n");

  for (int i=0; i<NPACK; i++) {
    Address client_address = { 0 };
    uint16_t client_port = 0;
    if (SocketReceiveFrom(s, buf, BUFLEN, 0, &client_address, &client_port) != SOCKETRESULT_OK) {
      diep("recvfrom()");
    }

    char* addr = AddressToIPString(&client_address);

    printf("Received packet from %s:%d\nData: %s\n\n", addr, ntohs(client_port), buf);

    free(addr);
  }

  close(s);
  return 0;
}
