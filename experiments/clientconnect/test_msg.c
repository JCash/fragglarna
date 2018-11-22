
#include "message.h"
#include <stdio.h>

int has_error = 0;


int main(int argc, char const *argv[])
{
	const int SIZE = 16;
	int8_t data_i8[SIZE];
	uint8_t data_u8[SIZE];
	for (int i = 0; i < SIZE; ++i)
	{
		data_i8[i] = i & 1 ? -i : i;
		data_u8[i] = i;
	}

	{
		Message msg_i8;
		Message_Init(&msg_i8, (uint8_t*)data_i8, sizeof(data_i8));
		Message_BeginRead(&msg_i8);

		for (int i = 0; i < SIZE; ++i)
		{
			int8_t expected = (int8_t)( i & 1 ? -i : i );
			int8_t value = Message_ReadInt8(&msg_i8);
			int ok = (expected == value);
			has_error |= !ok;
			printf("i8: %d  %d == %d == %s\n", i, expected, value, ok ? "true": "FALSE" );
		}
		has_error |= msg_i8.cursor != 16;
	}
	{
		Message msg_u8;
		Message_Init(&msg_u8, data_u8, sizeof(data_u8));
		Message_BeginRead(&msg_u8);

		for (int i = 0; i < SIZE; ++i)
		{
			uint8_t expected = (uint8_t)i;
			uint8_t value = Message_ReadUInt8(&msg_u8);
			int ok = (expected == value);
			has_error |= !ok;
			printf("u8: %d  %d == %d == %s\n", i, expected, value, ok ? "true": "FALSE" );
		}
		has_error |= msg_u8.cursor != 16;
	}

	return has_error;
}
