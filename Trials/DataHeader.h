#pragma once

#ifndef DataHeader00
#define DataHeader00
#include <string>


//Todo: Create Mapping of headers to handlers
//Then we can add in handle functions to the dataheader
namespace Trials
{
	struct DataHeader
	{
	public:
		char type_;							// what type of packet 
		unsigned int dataLength_;			// how long the data is

	public:
		DataHeader(char type, int dataLength);
	};

	struct ImageHeader : DataHeader
	{
	public:
		int resolutionX_, resolutionY_, uniqueId_;

		bool isCompressed_;
		std::string formatType_;

	public:
		ImageHeader(int resolutionX, int resolutionY, int uniqueId,
			bool isCompressed, int dataLength);
	};
}
#endif //DataHeader00