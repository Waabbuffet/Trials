#include "DataHeader.h"

namespace Trials
{
	DataHeader::DataHeader(char type, int dataLength, char* data) 
		: type_(type), 
		dataLength_(dataLength),
		data_(data)
	{

	}

	ImageHeader::ImageHeader(int resolutionX, int resolutionY, int uniqueId, bool isCompressed, int dataLength, char* data)
		: resolutionX_(resolutionX),
		resolutionY_(resolutionY),
		uniqueId_(uniqueId),
		isCompressed_(isCompressed),
		DataHeader{ 'F', dataLength, data}
	{

	}
}