#pragma once

#ifndef DataHeader00
#define DataHeader00
#include <string>
#include "Main.h";
#include "evpp/buffer.h"
#include "opencv2/opencv.hpp"

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

		bool isCompressed_, isEncoded_;
		

	public:
		ImageHeader(int resolutionX, int resolutionY, int uniqueId,
			bool isCompressed, int dataLength);
	};

	struct FaceHeader : DataHeader
	{
		unsigned int numberOfFaces;
	
		FaceHeader(unsigned int numberOfFaces, int dataLength);
	};

	struct PacketTracker
	{
		unsigned int headerSize;
		unsigned int bytesReceived;
		bool hasHeader;

		evpp::Buffer* header_buffer;
		evpp::Buffer* data_buffer;
		cv::Mat* image_buffer;

		PacketTracker();
	};
}
#endif //DataHeader00