#include "Client.h"

	namespace Trials 
	{
		class Client
		{
		private:
			evpp::EventLoop* loop_;
			std::string name_;
			
			int sessionCount_;
			std::shared_ptr<evpp::EventLoopThreadPool> tpool_;
			std::vector<std::shared_ptr<TCPSession>> sessions_;
			std::vector< std::function<void(const evpp::TCPConnPtr & conn, evpp::Buffer * buf)>> allSessionCallback_;
		public:
			Client(evpp::EventLoop* loop,
				const std::string& name,
				const std::string& serverAddr, // ip:port
				int sessionCount,	//How many sessions
				int threadCount,
				std::vector< std::function<void(const evpp::TCPConnPtr & conn, evpp::Buffer * buf)>> sessionCallback) :  //How many threads will manage each session
				name_(name),
				sessionCount_(sessionCount),
				allSessionCallback_(sessionCallback)
			{
				tpool_.reset(new evpp::EventLoopThreadPool(loop, threadCount));
				tpool_->Start(true);

				for (int i = 0; i < sessionCount; ++i)
				{
					TCPSession* session = new TCPSession(tpool_->GetNextLoop(), serverAddr, name, i, this);
					session->start();
					sessions_.push_back(std::shared_ptr<TCPSession>(session));
					try
					{
						auto callback = allSessionCallback_.at(i);
						session->setOnFunctionCallback(callback);
					}
					catch (std::out_of_range error)
					{
						std::cout << "[Trials.Client] Not enough sessions for the callback function at" << i << "\n";
					}
				}
			}

			~Client()
			{

			}

			void startAll()
			{
				for (int i = 0; i < sessionCount_; ++i)
				{
					sessions_[i]->start();
				}
			}

			void stopAll()
			{
				for (int i = 0; i < sessionCount_; ++i) 
				{
					sessions_[i]->stop();
				}
			}

			void writeToServer(int sessionId)
			{
				try
				{
					auto session = sessions_[sessionId];
					//TODO: Add in the write to server from session side 
				}
				catch (std::out_of_range error)
				{
					std::cout << "[Trials.Client] Tried to write to a session that does not exist\n";
				}
			}
		};
	}