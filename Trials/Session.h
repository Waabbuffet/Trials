#pragma once

#ifndef trials_session00
#define trials_session00
#include "Main.h"

#include <evpp/tcp_client.h>
#include <evpp/event_loop_thread_pool.h>
#include <evpp/buffer.h>
#include <evpp/tcp_conn.h>
#include <evpp/timestamp.h>

#include "PacketSerializer.h"
#include "Client.h"

namespace Trials
{
	class TCPSession : std::enable_shared_from_this<TCPSession>
	{
	public:
		TCPSession(evpp::EventLoop* loop,
			const std::string& serverAddr,
			const std::string& name,
			const int sessionId,
			Client* master_client);

		~TCPSession();

	public :
		void start();
		void stop();
		void setOnFunctionCallback(std::function<void(const evpp::TCPConnPtr & conn, evpp::Buffer * buf)> onMessageFunction);

	};
}
#endif //trials_session
