#include "message.h"

void Message_Init(Message* message, void* data, uint32_t datasize)
{
    message->data       = (uint8_t*)data;
    message->datasize   = datasize;
    message->cursor     = 0;
}

void Message_BeginRead(Message* message)
{
    message->cursor = 0;
}

static int Message_ReadBits(Message* message, int bits)
{
    int make_signed = bits < 0;
    bits = bits < 0 ? -bits : bits;

    int value = 0;

    // Currently, we can only read whole bytes
    if (bits == 8) {
        value = *(uint8_t*)&message->data[message->cursor];
    } else if (bits == 16) {
        value = *(uint16_t*)&message->data[message->cursor];
    } else if (bits == 32) {
        value = *(uint32_t*)&message->data[message->cursor];
    }
    message->cursor += bits / 8;

    if (make_signed)
    {
        if (value & (1 << (bits -1)))
        {
            value |= -1 ^ ( ( 1 << bits ) - 1 );
        }
    }
    return value;
}

const char* Message_ReadStringLine(Message* message)
{
    static char str[1024];
    char* c = (char*)&message->data[message->cursor];
    int remaining = message->datasize - message->cursor;
    int len = 0;
    while (remaining > 0 && len < (sizeof(str)-1) && *c != '\0' && *c != '\n')
    {
        str[len++] = *c++;
    }
    str[len] = 0;
    return (const char*)str;
}

int8_t Message_ReadInt8(Message* message)
{
    return (int8_t)Message_ReadBits(message, -8);
}
int16_t Message_ReadInt16(Message* message)
{
    return (int16_t)Message_ReadBits(message, -16);
}
int32_t Message_ReadInt32(Message* message)
{
    return (int32_t)Message_ReadBits(message, -32);
}
uint8_t Message_ReadUInt8(Message* message)
{
    return (uint8_t)Message_ReadBits(message, 8);
}
uint16_t Message_ReadUInt16(Message* message)
{
    return (uint16_t)Message_ReadBits(message, 16);
}
uint32_t Message_ReadUInt32(Message* message)
{
    return (uint32_t)Message_ReadBits(message, 32);
}
float Message_ReadFloat32(Message* message)
{
    return (float)Message_ReadBits(message, 32);
}
