#pragma once

#ifndef trials_session00
#define trials_session00
#include "Main.h"

#include <evpp/event_loop_thread_pool.h>
#include <evpp/buffer.h>
#include <evpp/tcp_conn.h>
#include <evpp/timestamp.h>
#include "DataHeader.h"

namespace Trials
{
	class TCPSession 
	{
	private:
		evpp::TCPClient client_;    //The evpp tcp client

		evpp::Buffer header_buffer;   //Used to store the incoming header
		evpp::Buffer data_buffer;     //Used to store the incoming data

		//Client* owner_;

		bool packetPending, hasHeader;
		std::mutex mutex_;          //The mutex locking the active_connection. Prevents changing the active_connection while we are writing to it 
		evpp::TCPConnPtr active_connection_; //The active connection, equivalent to a event loop and a socket
		std::function<void(const evpp::TCPConnPtr & conn, evpp::Buffer * buf)> onMessageFunction;

	public:
		TCPSession(evpp::EventLoop* loop,
			const std::string& serverAddr,
			const std::string& name,
			const int sessionId);

		~TCPSession();

		void start();
		void stop();
		bool hasPacketPending();
		bool isConnected();
		void setPacketPending(bool packetPending);
		void setOnFunctionCallback(std::function<void(const evpp::TCPConnPtr & conn, evpp::Buffer * buf)> onMessageFunction);
		void rawWriteToConnection(evpp::Buffer* buf);

	private:
		void onConnectionEstablished(const evpp::TCPConnPtr& conn);
		void defaultOnMessageFromServer(const evpp::TCPConnPtr& conn, evpp::Buffer* buf);
		void onMessageFromServer(const evpp::TCPConnPtr& conn, evpp::Buffer* buf);

	};
}
#endif //trials_session
