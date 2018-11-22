// Good reads
// https://github.com/id-Software/Quake-III-Arena/blob/master/code/server/sv_client.c

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
#include "message.h"

#define BUFLEN 512
#define NPACK 10

void diep(char *s)
{
  perror(s);
  exit(1);
}

const int MAX_CLIENTS = 4;

enum ClientState
{
    CS_FREE,
    CS_CONNECTED,
};

typedef struct Client
{
    Address address;
    int     port;
    int     state;
} Client;

typedef struct Server
{
    Socket socket;
    Client clients[MAX_CLIENTS];
} Server;

static void SendServerCommand(Server* server, Client* client, const char* cmd)
{
    int sent_bytes = 0;
    SocketSendTo(server->socket, cmd, strlen(cmd), &sent_bytes, &client->address, client->port);
}

static void DropClient(Server* server, Client* client, const char* reason)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "disconnect \"%s\"", reason);
    SendServerCommand(server, client, buf);
    memset(client, 0, sizeof(Client));
}

static void ConnectClient(Server* server, Address* from, int port)
{
    int n = 0;
    Client* client = server->clients;
    Client* found = 0;
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (memcmp(&client->address, from, sizeof(Address)) == 0)
        {
            found = client;
            break;
        }
        if (client->state == CS_FREE)
        {
            found = client;
        }
        client++;
    }

    if (!found)
    {
        // Too many connections
        return;
    }

    if (found->state == CS_CONNECTED)
    {
        // It was already connected, skip
    }
    else if(found)
    {
        client = found;
        memcpy(&client->address, from, sizeof(Address));
        client->state = CS_CONNECTED;
        client->port = port;

        char* addr = AddressToIPString(from);
        int id = (int)(client - server->clients);
        printf("Connection from %s:%d on client %d\n", addr, port, id);
        printf("client %d ( %p): %d  %d\n\n", id, client, client->state, CS_CONNECTED);

        free(addr);
    }
}

static void HandleConnectionlessMessage(Server* server, Message* message, Address* from, int port)
{
    Message_BeginRead(message);
    Message_ReadUInt32(message); // skip the control bytes
    const char* cmd = Message_ReadStringLine(message);

    printf("cmd: %s\n", cmd);

    if (strcmp(cmd, "connect") == 0) {
        ConnectClient(server, from, port);
    }
}

static int IsConnectionLessMessage(Message* message)
{
    // If the first bytes are 0xFFFFFFFF, it's a separate command
    return message->datasize >= 4 && 0xFFFFFFFF == *(uint32_t*)message->data;
}

static void HandleNetworkMessage(Server* server, Message* message, Address* from, int port)
{
    //
    if (IsConnectionLessMessage(message))
    {
        HandleConnectionlessMessage(server, message, from, port);
        return;
    }

    // Do some real state handling
}

static void Tick(Server* server)
{
    static int tick = 0;
    Client* client = server->clients;
    for (int i = 0; i < MAX_CLIENTS; ++i, ++client)
    {
        //printf("client %d ( %p): %d  %d\n", i, client, client->state, CS_CONNECTED);
        if (client->state != CS_CONNECTED)
            continue;

        char buf[BUFLEN];
        snprintf(buf, sizeof(buf), "Tick %d", tick);
        printf("%s", buf);

        if (SocketSendTo(server->socket, buf, strlen(buf), 0, &client->address, client->port) != SOCKETRESULT_OK)
        {
            diep("sendto()");
        }
    }

    const char* dial = "|/-\\";
    printf("\b%c", dial[(tick/12)%4]);
    fflush(stdout);
    ++tick;
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

    Server server = {0};
    server.socket = s;

    printf("\nServer started...\n");

    uint32_t sleepms = 1000 / 60;

    while (true)
    {
        r = SocketSleep(s, sleepms);
        if (r == SOCKETRESULT_OK)
        {
            Address client_address = { 0 };
            uint16_t client_port = 0;
            char buf[BUFLEN];
            int received_bytes = 0;
            if (SocketReceiveFrom(s, buf, BUFLEN, &received_bytes, &client_address, &client_port) != SOCKETRESULT_OK) {
                diep("recvfrom()");
            }

            char* addr = AddressToIPString(&client_address);

            printf("Received packet from %s:%d\nData: %s\n", addr, ntohs(client_port), buf);

            Message msg;
            Message_Init(&msg, (uint8_t*)buf, received_bytes);
            HandleNetworkMessage(&server, &msg, &client_address, client_port);

            free(addr);
        }
        else if(r == SOCKETRESULT_FAIL)
        {
            printf("Error, aborting\n");
            break;
        }

        Tick(&server);
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
