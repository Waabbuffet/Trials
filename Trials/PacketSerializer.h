#pragma once

#ifndef PacketSerializer00 
#define PacketSerializer00
#include "Main.h"
#include "evpp/buffer.h"

namespace Trials
{
	evpp::Buffer* createPacket(void* hStart, int hLength);
	void createPacket(evpp::Buffer* emptyBuffer, void* hStart, int hLength);
}

#endif;

