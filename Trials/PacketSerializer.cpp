#include "PacketSerializer.h"


namespace Trials
{
	//This function will create a new packet and will create the buffer for you as well
	//Note if you don't free the memory you will cause a memory leak
	//Requires:
	//- Hstart - a pointer pointing to the starting location of the data you want to send
	//- Hlength - the length of the data you want to send
	evpp::Buffer* createPacket(void* hStart, int hLength)
	{
		evpp::Buffer* thebuffer = new evpp::Buffer(hLength, 0);
		createPacket(thebuffer, hStart, hLength);

		return thebuffer;
	}

	void createPacket(evpp::Buffer* emptyBuffer, void* hStart, int hLength)
	{
		emptyBuffer->Write(hStart, hLength);
	}	
}