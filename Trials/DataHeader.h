#pragma once

#ifndef DataHeader00
#define DataHeader00
#include "Main.h"

//Todo: Create Mapping of headers to handlers
//Then we can add in handle functions to the dataheader
namespace Trials
{
	struct DataHeader
	{
	public:
		char type_;			// what type of packet 
		int dataLength_;	// how long the data is
		char* data_;		// where the data is

	public:
		DataHeader(char type, int dataLength, char* data);
	};

	struct ImageHeader : DataHeader
	{
	public:
		int resolutionX_, resolutionY_, uniqueId_;

		bool isCompressed_;
		std::string formatType_;

	public:
		ImageHeader(int resolutionX, int resolutionY, int uniqueId,
			bool isCompressed, int dataLength, char* data);
	};
}
#endif //DataHeader00