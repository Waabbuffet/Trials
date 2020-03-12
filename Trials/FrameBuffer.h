#pragma once

#ifndef FRAMEBUFFER_00
#define FRAMEBUFFER_00

#include "opencv2/core/mat.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include <mutex>
#include <cstdlib>
#include <vector>
#include <chrono>

namespace Trials
{
	class Server;

	class FrameBuffer
	{
	public:
		FrameBuffer(unsigned int maxFrames, unsigned int waitForXFrames, unsigned int pollFrames);

		void run();

		bool addFrame(cv::Mat* framePtr, unsigned int clientId);

		void setOwner(Server* theOwner);
		
	private:
		
		cv::CascadeClassifier face_cascade, eyes_cascade;

		int calculateFrameDifference();

		int modulus(int a, int b);
		
		void playFrame();

		void onFrameReceived(cv::Mat *decoded, unsigned connectionId);

		void onFaceReceived(cv::Rect face);
		
		unsigned int maxFrames, currentFrame, endOfBuffer, waitForXFrames, pollFrames;

		std::mutex frameBufferLock;

		Server* theOwner;
		std::vector<std::pair<unsigned int, cv::Mat*>> frame_buffer;
	};
}
#endif // FRAMEBUFFER_00
