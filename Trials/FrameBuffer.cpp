#include "FrameBuffer.h"
#include "Server.h"

namespace Trials
{
	FrameBuffer::FrameBuffer(unsigned int maxFrames, unsigned int waitForXFrames, unsigned int pollFrames)
		: maxFrames(maxFrames), 
		  frame_buffer(maxFrames),
		  currentFrame(0),
		  endOfBuffer(0),
	      waitForXFrames(waitForXFrames),
		  pollFrames(pollFrames)
	{

	}

	bool FrameBuffer::addFrame(cv::Mat* framePtr)
	{
		int end = (endOfBuffer + 1) % maxFrames;
		std::cout << "Added Frame" << endOfBuffer << "vs " <<end << "endOfBuffer" << " current: " << currentFrame;
		if (end != currentFrame)
		{
			frame_buffer[endOfBuffer] = framePtr;
			endOfBuffer = end;
			return true;
		}
		return false;
	}

	//Example:
	// 1, 6 = 1
	// -1 ,6 = 5
	int FrameBuffer::modulus(int a, int b)
	{
		if (a > 0)
		{
			return a % b;
		}
		else
		{
			return b - std::abs(a % b);
		}
	}

	int FrameBuffer::calculateFrameDifference()
	{
		if (endOfBuffer == currentFrame)
		{
			std::cout << "Calculate Frame Difference hehsn";
			return 0;
		}
		else
		{
			int difference = endOfBuffer - currentFrame;
			std::cout << "Calculate Frame buffer" << difference;
			return modulus(difference, maxFrames);
		}
	}

	//will display and auto increment to the next frame
	void FrameBuffer::playFrame()
	{
		auto theFrame = frame_buffer[currentFrame];
	
		if (theFrame)
		{
			if (theFrame->type() == CV_8UC1)
			{
				cv::Mat decoded = cv::imdecode(*theFrame, 1);

				if (decoded.data != NULL)
				{
					cv::imshow("Frame Output", decoded);
					cv::waitKey(30);
					Server::pool.disown(theFrame);
					currentFrame = (currentFrame + 1) % (maxFrames - 1);
				}
				else
				{
					std::cout << "The data was null ";
				}
			}
			else if (theFrame->type() == CV_8UC3)
			{
				cv::imshow("Frame Output", *theFrame);
				cv::waitKey(10);
				Server::pool.disown(theFrame);
				currentFrame = (currentFrame + 1) % (maxFrames - 1);
			}
		}
		else
		{
			std::cout << "Tried to play non-existent frame"; 
		}
	}

	void FrameBuffer::run()
	{
		while (true)
		{
			//Calc how many frames we can display
			int frameDifference = calculateFrameDifference();
			std::cout << "Frame end:" << frameDifference <<  " VS " << waitForXFrames << " currentFrame: " << currentFrame <<std::endl;

			if (frameDifference >= waitForXFrames)
			{
				playFrame();
			}
			else
			{
				//Welp just wait until something comes in
				std::this_thread::sleep_for(std::chrono::milliseconds(pollFrames));
			}
		}
	}
}
