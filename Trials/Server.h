#pragma once

#ifndef trials_server00
#define trials_server00

#include <iostream>
#include <stdexcept>
#include <set>

#include "Main.h"
#include "DataHeader.h"
#include <evpp/buffer.h>
#include <evpp/tcp_conn.h>
#include <evpp/tcp_server.h>
#include <evpp/timestamp.h>
#include <evpp/event_loop_thread_pool.h>

#include "HeapObjectPool.h"
#include "opencv2/core/mat.hpp"

namespace Trials
{
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

	class Server
	{
	private:
		std::set<evpp::TCPConnPtr> connectionPools;
		evpp::TCPServer theServer;

		//std::vector<evpp::Buffer*> header_buffers;   //Used to store the incoming header
		//std::vector<evpp::Buffer*> data_buffers;     //Used to store the incoming data
		//std::vector<cv::Mat> image_matrix;			 //Used to store the incoming matrix image

		std::vector<PacketTracker*> connectionData;

		HeapObjectPool<cv::Mat, 128> pool = HeapObjectPool<cv::Mat, 128>();
	public:
		Server(evpp::EventLoop* loop,
			const std::string& name,
			const std::string& serverAddr, // ip:port
			int threadCount);

		void Start();

	private: 
		void OnConnection(const evpp::TCPConnPtr& conn);
		void onMessageReceive(const evpp::TCPConnPtr& conn, evpp::Buffer* buf);

		void handleImagePacket(const evpp::TCPConnPtr& conn, evpp::Buffer* buf, ImageHeader*headers);
		void handleDataPacket(const evpp::TCPConnPtr& conn, evpp::Buffer* buf, DataHeader* headers);
	};
}
#endif
