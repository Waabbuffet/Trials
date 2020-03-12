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

		if (!face_cascade.load(".\\datasets\\cascades\\haarcascade_frontalface_default.xml"))
		{
			throw std::exception("Could not find the xml file");
		}
	}

	bool FrameBuffer::addFrame(cv::Mat* framePtr, unsigned int clientId)
	{
		frameBufferLock.lock();
		int end = (endOfBuffer + 1) % maxFrames;
		
		if (end != currentFrame)
		{
			frame_buffer[endOfBuffer] = std::pair<unsigned int, cv::Mat*>(clientId, framePtr);
			endOfBuffer = end;
			frameBufferLock.unlock();
			return true;
		}
		frameBufferLock.unlock();
		return false;
	}

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

	void FrameBuffer::setOwner(Server* Owner)
	{
		theOwner = Owner;
	}

	int FrameBuffer::calculateFrameDifference()
	{
		if (endOfBuffer == currentFrame)
		{
			return 0;
		}
		else
		{
			int difference = endOfBuffer - currentFrame;
			return modulus(difference, maxFrames);
		}
	}

	//will display and auto increment to the next frame
	void FrameBuffer::playFrame()
	{
		frameBufferLock.lock();
		auto thePair = frame_buffer[currentFrame];
		auto theFrame = thePair.second;
		frameBufferLock.unlock();
		if (theFrame)
		{
			if (theFrame->type() == CV_8UC1)
			{
				cv::Mat decoded = cv::imdecode(*theFrame, 1);
				Server::pool.disown(theFrame);

				if (decoded.data != NULL)
				{
					onFrameReceived(&decoded, thePair.first);
					
					currentFrame = (currentFrame + 1) % (maxFrames - 1);
				}
				else
				{
					std::cout << "The data was null ";
				}
			}
			else if (theFrame->type() == CV_8UC3)
			{
				onFrameReceived(theFrame, thePair.first);
				Server::pool.disown(theFrame);
				currentFrame = (currentFrame + 1) % (maxFrames - 1);
			}
		}
		else
		{
			//std::cout << "Tried to play non-existent frame"; 
		}
	}

	void FrameBuffer::run()
	{
		while (true)
		{
			//frameBufferLock.lock();
			int frameDifference = calculateFrameDifference();
			//frameBufferLock.unlock();
			
			if (frameDifference >= waitForXFrames)
			{
				playFrame();
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(pollFrames));
			}
		}
	}

	void FrameBuffer::onFrameReceived(cv::Mat *decoded, unsigned int connectionId)
	{
		cv::Mat frame_gray;
		std::vector<cv::Rect> faces;

		cv::cvtColor(*decoded, frame_gray, cv::COLOR_BGR2GRAY);
		cv::equalizeHist(frame_gray, frame_gray);

		face_cascade.detectMultiScale(frame_gray, faces, 1.2, 10, 0, cv::Size(20,20));
		std::string theMessage = std::to_string(faces.size())+":";
		for (size_t i =0; i < faces.size(); i ++)
		{
			cv::rectangle(*decoded, faces[i], cv::Scalar(255,255,255), 5);
			onFaceReceived(faces[i]);
			theMessage += std::to_string(faces[i].x+ faces[i].width/2) + ":";
		}
		std::cout << "Ignoring the message" << theMessage << std::endl;

		FaceHeader header(faces.size, theMessage.length());
		evpp::Buffer theBuffer;

		theBuffer.Write(&header, sizeof(header));
		theBuffer.Write(theMessage.data(), theMessage.length());

		theOwner->SendPacketToClient(&theBuffer, connectionId);

		cv::imshow("Frame output", *decoded);
		cv::waitKey(5);
	}

	void FrameBuffer::onFaceReceived(cv::Rect face)
	{
		//Here we can send back the coordinates to the client
		//theOwner->SendPacketToClient();

	}
}
