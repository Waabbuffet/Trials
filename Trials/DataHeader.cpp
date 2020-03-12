#include "DataHeader.h"

namespace Trials
{
	DataHeader::DataHeader(char type, int dataLength) 
		: type_(type), 
		dataLength_(dataLength)
	{

	}

	ImageHeader::ImageHeader(int resolutionX, int resolutionY, int uniqueId, bool isCompressed, int dataLength)
		: resolutionX_(resolutionX),
		resolutionY_(resolutionY),
		uniqueId_(uniqueId),
		isCompressed_(isCompressed),
		DataHeader{ 'F', dataLength}
	{

	}

	FaceHeader::FaceHeader(unsigned int numberOfFaces, int dataLength)
		: numberOfFaces(numberOfFaces),
		DataHeader{ 'B', dataLength}
	{

	}

	PacketTracker::PacketTracker()
		:
		bytesReceived(0),
		hasHeader(false),
		header_buffer(new evpp::Buffer(sizeof(Trials::DataHeader))),
		data_buffer(new evpp::Buffer(sizeof(Trials::DataHeader))),
		headerSize(sizeof(Trials::DataHeader))
	{

		
	}
}