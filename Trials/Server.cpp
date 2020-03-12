#include "Server.h"
#include "opencv2/opencv.hpp"

namespace Trials
{
    Server::Server(evpp::EventLoop* loop,
        const std::string& serverAddr,
        const std::string& name,
        int threadCount)
        : theServer(loop, serverAddr, name, threadCount),
        connectionPools(),
        connectionData()
    {
        theServer.SetConnectionCallback(
            std::bind(&Server::OnConnection, this, std::placeholders::_1));

        theServer.SetMessageCallback(
            std::bind(&Server::onMessageReceive, this, std::placeholders::_1, std::placeholders::_2));
        frame.setOwner(this);
    }

    void Server::Start()
    {
        theServer.Init();
        theServer.Start();
    }

    void Server::OnConnection(const evpp::TCPConnPtr& conn)
    {
        if (conn->IsConnected())
        {
            connectionPools.insert(conn);

            if (conn->id() > connectionData.size())
            {
                connectionData.push_back(new PacketTracker());
            }
        }
        else
        {
            connectionPools.erase(conn);
        }
    }

    void Server::onMessageReceive(const evpp::TCPConnPtr& conn, evpp::Buffer* buf)
    {
        unsigned int tempId = conn->id()-1;
        //std::cout << "Server received something "<< tempId << "Remember: "<< connectionData.size() <<std::endl;

        evpp::Buffer* header_buffer = connectionData[tempId]->header_buffer;
        evpp::Buffer* data_buffer = connectionData[tempId]->data_buffer;
        bool byPass = connectionData[tempId]->hasHeader;

        while (buf->size() > 0)
        {
           //std::cout << "Server received something " << tempId << "Server read" << buf->size() << std::endl;
            if (!connectionData[tempId]->hasHeader)//is used for checking if we are collecting data, or are we collecting the header 
            {
                if (header_buffer->size() < connectionData[tempId]->headerSize)
                {
                    //super important that this never goes negative otherwise ill be here forever
                    
                    header_buffer->Append(buf->Next(connectionData[tempId]->headerSize - header_buffer->size()));
                    //std::cout << "After buffer" << buf->size() << std::endl;
                }

                if (header_buffer->size() == sizeof(Trials::DataHeader))
                {
                    struct Trials::DataHeader* incoming_header = (Trials::DataHeader*)header_buffer->data();
                    if (incoming_header != nullptr)
                    {
                        switch (incoming_header->type_)
                        {
                        case 'F':
                           
                            connectionData[tempId]->headerSize = sizeof(ImageHeader);
                            break;
                        case 'D':
                            //Otherwise this is what the client wanted to send us
                            connectionData[tempId]->hasHeader = true;
                            connectionData[tempId]->bytesReceived = 0;  
                            break;
                        }
                    }
                    else
                    {
                        //Reset function
                        //TODO: Move this to a function
                        std::cout << "[TCPSession] Unable to read header";
                        connectionData[tempId]->hasHeader = false;
                        connectionData[tempId]->bytesReceived = 0;
                        connectionData[tempId]->headerSize = sizeof(DataHeader);
                        header_buffer->Reset();
                    }               
                }

                if (header_buffer->size() == connectionData[tempId]->headerSize)
                {
                    connectionData[tempId]->hasHeader = true;
                    connectionData[tempId]->bytesReceived = 0;
                }
            }

            if (connectionData[tempId]->hasHeader)//now we have the full header
            {
                try
                {
                    struct Trials::DataHeader* incoming_header = (Trials::DataHeader*)header_buffer->data();
                    if (incoming_header != nullptr)
                    {
                       //TODO: Split this into handle packet read, and handle packet complete
                        switch (incoming_header->type_)
                        {
                        case 'F'://We know this is a type F
                            handleImagePacket(conn, buf, (Trials::ImageHeader*)incoming_header);
                            break;
                        case 'D':
                            handleDataPacket(conn, buf, incoming_header);
                            break;
                        }

                        if (connectionData[tempId]->bytesReceived == incoming_header->dataLength_)
                        {
                            connectionData[tempId]->hasHeader = false;
                            connectionData[tempId]->bytesReceived = 0;
                            connectionData[tempId]->headerSize = sizeof(DataHeader);
                            header_buffer->Reset();
                        }
                    }
                    else
                    {
                        std::cout << "[TCPSession] Unable to read header due to typecase";
                        connectionData[tempId]->hasHeader = false;
                        connectionData[tempId]->bytesReceived = 0;
                        connectionData[tempId]->headerSize = sizeof(DataHeader);
                        header_buffer->Reset();
                    }
                }
                catch (std::exception e)
                {
                    std::cout << "[TCPSession] Unable to read header due to e" << e.what();
                    connectionData[tempId]->hasHeader = false;
                    connectionData[tempId]->bytesReceived = 0;
                    connectionData[tempId]->headerSize = sizeof(DataHeader);
                    header_buffer->Reset();
                }
            }
        }
    }

    //If we are sending a decoded frame, the size isnt x by y, its, 1 by size of bufffer
    void Server::handleImagePacket(const evpp::TCPConnPtr& conn, evpp::Buffer* buf, ImageHeader* headers)
    {
        unsigned int tempId = conn->id() - 1;
        //TODO: this changed from being resolutionX, resolution Y to datalength_, 1, CV_8UC1
        Trials::OpenCVMatrixRequirement require_(headers->dataLength_, 1, 3, CV_8UC1);
        if (!headers->isEncoded_) //Its raw and we must read raw data
        {
            require_.sizeX = headers->resolutionX_;
            require_.sizeY = headers->resolutionY_;
            require_.type = CV_8UC3;
        }

        //std::cout << "The header resolution is: " << headers->resolutionX_ << "resolutionY: " << headers->resolutionY_;
        if (connectionData[tempId]->bytesReceived == 0) //Need to grab matrix from pool
        {
            connectionData[tempId]->image_buffer = pool.acquire(&require_);
        }
        
        //std::cout << "current: " << connectionData[tempId]->bytesReceived << "vs" << headers->dataLength_ <<std::endl;
        if (connectionData[tempId]->image_buffer != nullptr) //if the pool return an image buffer now we can read data
        {
            evpp::Slice readSlice = buf->Next(headers->dataLength_ - connectionData[tempId]->bytesReceived);
            unsigned int bytes_read = readSlice.size();

            //I can't remove this memcopy I don't think 
            std::memcpy((unsigned char*)((unsigned char*)connectionData[tempId]->image_buffer->data + connectionData[tempId]->bytesReceived), (unsigned char*)readSlice.data(), readSlice.size());
            connectionData[tempId]->bytesReceived += bytes_read;
            
        }//if not successful wait until next read 

        if (connectionData[tempId]->bytesReceived == headers->dataLength_)
        {
            //Here we can move the image to the frame buffer queue 
            frame.addFrame(connectionData[tempId]->image_buffer, tempId);
        }
    }

    void Server::handleDataPacket(const evpp::TCPConnPtr& conn, evpp::Buffer* buf, DataHeader* headers)
    {
        //NO-OP yet
    }

    void Server::SendPacketToClient(evpp::Buffer* buf, unsigned int uniqueId)
    {
 
        for (auto conn : connectionPools)
        {
            if (conn->id() == uniqueId)
            {
                conn->Send(buf);
            }
        }
    }
}
