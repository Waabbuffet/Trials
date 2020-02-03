#include "Client.h"

namespace Trials 
{
	Client::Client(evpp::EventLoop* loop,
		const std::string& name,
		const std::string& serverAddr, // ip:port
		int sessionCount,	//How many sessions
		int threadCount) :  //How many threads will manage each session
		loop_(loop),
		name_(name),
		sessionCount_(sessionCount)
	{
		tpool_.reset(new evpp::EventLoopThreadPool(loop, threadCount));
		tpool_->Start(true);

		for (int i = 0; i < sessionCount; ++i)
		{
			TCPSession* session = new TCPSession(tpool_->GetNextLoop(), serverAddr, name, i);
			session->start();
			sessions_.push_back(session);
		}
	}

	Client::Client(evpp::EventLoop* loop,
		const std::string& name,
		const std::string& serverAddr, // ip:port
		int sessionCount,	//How many sessions
		int threadCount,
		std::vector< std::function<void(const evpp::TCPConnPtr & conn, evpp::Buffer * buf)>> sessionCallback) :  //How many threads will manage each session
		loop_(loop),
		name_(name),
		sessionCount_(sessionCount),
		allSessionCallback_(sessionCallback)
	{
		tpool_.reset(new evpp::EventLoopThreadPool(loop, threadCount));
		tpool_->Start(true);

		for (int i = 0; i < sessionCount; ++i)
		{
			TCPSession* session = new TCPSession(tpool_->GetNextLoop(), serverAddr, name, i);
			session->start();
			sessions_.push_back(session);
			try
			{
				auto callback = allSessionCallback_.at(i);
				if(callback != NULL)
					session->setOnFunctionCallback(callback);
			}
			catch (std::out_of_range error)
			{
				std::cout << "[Trials.Client] Not enough sessions for the callback function at" << i << "\n";
			}
		}
	}

	Client::~Client()
	{

	}

	void Client::startAll()
	{
		for (int i = 0; i < sessionCount_; ++i)
		{
			sessions_[i]->start();
		}
	}

	void Client::stopAll()
	{
		for (int i = 0; i < sessionCount_; ++i) 
		{
			sessions_[i]->stop();
		}
	}
	
	bool Client::isSessionWaiting(int sessionId)
	{
		try
		{
			auto session = sessions_[sessionId];
			return session->hasPacketPending();
		}
		catch (std::out_of_range error)
		{
			std::cout << "[Trials.Client] Tried to write to a session that does not exist\n";
		}
		return true;
	}

	bool Client::isSessionConnected(int sessionId)
	{
		try
		{
			auto session = sessions_[sessionId];
			return session->isConnected();
		}
		catch (std::out_of_range error)
		{
			std::cout << "[Trials.Client] Tried to write to a session that does not exist\n";
		}
		return false;
	}

	void Client::waitForSessionConnection(int sessionId)
	{
		try
		{
			auto session = sessions_[sessionId];
			
			std::cout << "waiting for connection";

			while (!session->isConnected());

			std::cout << "Client is connected";

			return;
		}
		catch (std::out_of_range error)
		{
			std::cout << "[Trials.Client] Tried to write to a session that does not exist\n";
		}
	}

	void Client::writeToServer(int sessionId, evpp::Buffer* data_buffer)
	{
		try
		{
			auto session = sessions_[sessionId];
			session->setPacketPending(true);
			session->rawWriteToConnection(data_buffer);
		}
		catch (std::out_of_range error)
		{
			std::cout << "[Trials.Client] Tried to write to a session that does not exist\n";
		}
	}
}