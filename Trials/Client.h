#pragma once

#ifndef trials_client00
#define trials_client00

#include <iostream>
#include <stdexcept>
#include <vector>

#include "Session.h"
#include "DataHeader.h"
#include <evpp/event_loop_thread_pool.h>

namespace Trials 
{
	class Client
	{
	private:
		evpp::EventLoop* loop_;
		std::string name_;

		int sessionCount_;
		std::shared_ptr<evpp::EventLoopThreadPool> tpool_;
		std::vector<TCPSession*> sessions_;
		std::vector< std::function<void(const evpp::TCPConnPtr & conn, evpp::Buffer * buf)>> allSessionCallback_;

	public:
		Client(evpp::EventLoop* loop,
			const std::string& name,
			const std::string& serverAddr, // ip:port
			int sessionCount,	//How many sessions
			int threadCount);

		Client(evpp::EventLoop* loop,
			const std::string& name,
			const std::string& serverAddr, // ip:port
			int sessionCount,	//How many sessions
			int threadCount,
			std::vector< std::function<void(const evpp::TCPConnPtr & conn, evpp::Buffer * buf)>> sessionCallback);
		~Client();

		bool isSessionWaiting(int sessionId);
		bool isSessionConnected(int sessionId);
		void waitForSessionConnection(int sessionId);

		void writeToServer(int sessionId, evpp::Buffer* data_buffer);
		void startAll();
		void stopAll();
	};
}
#endif