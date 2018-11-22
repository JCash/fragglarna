#pragma once

#include "message.h"

struct NetChannel;

void NC_Transmit(NetChannel* channel, Message* message);
