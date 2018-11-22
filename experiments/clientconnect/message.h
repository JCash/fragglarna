#pragma once

#include <stdint.h>

/*

Connectionless message:
     E.g.  "connect;"
     A message that is prepended with four bytes (0xFFFFFFFF) and a command string

*/

typedef struct Message
{
    uint8_t* data;
    uint32_t cursor;
    uint32_t datasize;
} Message;


void Message_Init(Message* message, void* data, uint32_t datasize);
void Message_BeginRead(Message* message);

const char* Message_ReadStringLine(Message* message);
int8_t      Message_ReadInt8(Message* message);
int16_t     Message_ReadInt16(Message* message);
int32_t     Message_ReadInt32(Message* message);

uint8_t     Message_ReadUInt8(Message* message);
uint16_t    Message_ReadUInt16(Message* message);
uint32_t    Message_ReadUInt32(Message* message);

float       Message_ReadFloat32(Message* message);
