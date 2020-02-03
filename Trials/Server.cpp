#include "Server.h"
#include "opencv2/opencv.hpp"

namespace Trials
{

    PacketTracker::PacketTracker(): 
        bytesReceived(0),
        hasHeader(false),
        header_buffer(new evpp::Buffer(sizeof(Trials::DataHeader))),
        data_buffer(new evpp::Buffer(sizeof(Trials::DataHeader))), 
        headerSize(sizeof(Trials::DataHeader))
    {

    }

    //TODO: I'd love to make a generic version of this
    //Basically we can register packets to initializers, handlers, and finishers
    //Then I can use this as a base for anything
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
                std::cout << "Expanded the data" << conn->id() << std::endl;
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
       // std::cout << "Server received something "<< tempId << "Remember: "<< connectionData.size() <<std::endl;

        evpp::Buffer* header_buffer = connectionData[tempId]->header_buffer;
        evpp::Buffer* data_buffer = connectionData[tempId]->data_buffer;

        while (buf->size() >= sizeof(Trials::DataHeader))
        {
            if (!connectionData[tempId]->hasHeader)//is used for checking if we are collecting data, or are we collecting the header 
            {
                if (header_buffer->size() < connectionData[tempId]->headerSize)
                {
                    //super important that this never goes negative otherwise ill be here forever
                    //TODO: Catch this error and stop
                    header_buffer->Append(buf->Next(connectionData[tempId]->headerSize - header_buffer->size()));
                }

                if (header_buffer->size() == sizeof(Trials::DataHeader))
                {
                    struct Trials::DataHeader* incoming_header = (Trials::DataHeader*)header_buffer->data();
                    if (incoming_header != nullptr)
                    {
                        switch (incoming_header->type_)
                        {
                        case 'F':
                            //Here we promote the data into an image type and need to go back to collecting that
                            //TODO: We can save one loop cycle by looking for data now
                            std::cout << "Expanded the header size to" << sizeof(ImageHeader) << "from" << connectionData[tempId]->headerSize;
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

    void Server::handleImagePacket(const evpp::TCPConnPtr& conn, evpp::Buffer* buf, ImageHeader* headers)
    {
        unsigned int tempId = conn->id() - 1;
        Trials::OpenCVMatrixRequirement require_(headers->resolutionX_, headers->resolutionY_, 3, CV_8UC3); 

        if (connectionData[tempId]->bytesReceived == 0) //Need to grab matrix from pool
        {
            connectionData[tempId]->image_buffer = pool.acquire(&require_);
        }
        
        if (connectionData[tempId]->image_buffer != nullptr) //if the pool return an image buffer now we can read data
        {
            evpp::Slice readSlice = buf->Next(headers->dataLength_ - connectionData[tempId]->bytesReceived);
            int bytes_read = readSlice.size();

            //I can't remove this memcopy I don't think 
            std::memcpy((unsigned char*)((unsigned char*)connectionData[tempId]->image_buffer->data + connectionData[tempId]->bytesReceived), (unsigned char*)readSlice.data(), readSlice.size());
            connectionData[tempId]->bytesReceived += bytes_read;
        }//if not successful wait until next read 

        if (connectionData[tempId]->bytesReceived == headers->dataLength_)
        {
            //Here we can move the image to the frame buffer queue 
            //std::cout << "Read the image!" << connectionData[tempId]->bytesReceived;

           // cv::imshow("test", *connectionData[tempId]->image_buffer);

            //cv::waitKey(3000);

            pool.disown(connectionData[tempId]->image_buffer);
            connectionData[tempId]->image_buffer = nullptr;
        }
    }

    void Server::handleDataPacket(const evpp::TCPConnPtr& conn, evpp::Buffer* buf, DataHeader* headers)
    {
        //NO-OP yet
    }
}
