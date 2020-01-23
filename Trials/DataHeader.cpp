#include "DataHeader.h"

namespace Trials
{
	struct DataHeader
	{
		char type_;			// what type of packet 
		int dataLength_;	// how long the data is
		char* data_;		// where the data is

	public:
		DataHeader(char type, int dataLength, char* data) 
			: type_(type), 
			dataLength_(dataLength),
			data_(data)
		{

		}
	};

	struct ImageHeader : DataHeader
	{
		int resolutionX_, resolutionY_, uniqueId_;

		bool isCompressed_;
		std::string formatType_;
		
		ImageHeader(int resolutionX, int resolutionY, int uniqueId, bool isCompressed, int dataLength, char* data)
			: resolutionX_(resolutionX),
			resolutionY_(resolutionY),
			uniqueId_(uniqueId),
			isCompressed_(isCompressed),
			DataHeader{ 'F', dataLength, data}
		{

		}
	};
}