#pragma once

#ifndef FRAMEBUFFER_00
#define FRAMEBUFFER_00

#include "opencv2/core/mat.hpp"
#include "opencv2/opencv.hpp"

#include <cstdlib>
#include <vector>
#include <chrono>

namespace Trials
{
	class FrameBuffer
	{
	public:
		FrameBuffer(unsigned int maxFrames, unsigned int waitForXFrames, unsigned int pollFrames);

		//Eventually use analytics to play the frame
		void run();

		bool addFrame(cv::Mat* framePtr);
		
	private:
		
		int calculateFrameDifference();

		int modulus(int a, int b);
		
		void playFrame();

		unsigned int maxFrames, currentFrame, endOfBuffer, waitForXFrames, pollFrames;
		std::vector<cv::Mat*> frame_buffer;
	};
}
#endif // FRAMEBUFFER_00
