#include "Session.h"

namespace Trials 
{
    /* A Session is a single connection to the server. A client can have multiple sessions(connections) 
       to the server. An example is to send multiple frames at once, across multiple ports.
       As the image files become bigger it is very unlikely that we can send 1 frame before the next frame.
       
       A session allows a client to:
                    - Register callback functions
                    - Write to the socket, (never have to read due to the event loop)
    */
        TCPSession::TCPSession(evpp::EventLoop* loop,
            const std::string& serverAddr,
            const std::string& name,
            const int sessionId)
            : client_(loop, serverAddr, name),
            
            active_connection_(),
            header_buffer(sizeof(Trials::DataHeader)),
            data_buffer(sizeof(Trials::DataHeader)),
            sessionId(sessionId)
        { 
            client_.SetConnectionCallback(
                std::bind(&TCPSession::onConnectionEstablished, this, std::placeholders::_1));
            
            client_.SetMessageCallback(
                std::bind(&TCPSession::onMessageFromServer, this, std::placeholders::_1, std::placeholders::_2));
            client_.set_connecting_timeout(evpp::Duration(10.0));
            
        }

        TCPSession::~TCPSession()
        {

        }
        //This function will use the session tcp socket to send the contents of buf over to the server
        //Note raw means it will not do any preprocessing on the buffer before the contents are sent over 
        void TCPSession::rawWriteToConnection(evpp::Buffer* buf)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (active_connection_)
            {
                if (active_connection_->IsConnected())
                {
                    setPacketPending(true);
                    active_connection_->Send(buf);
                }
            }
        }

        bool TCPSession::hasPacketPending()
        {
            return this->packetPending;
        }

        void TCPSession::setPacketPending(bool packetPending)
        {
            this->packetPending = packetPending;
        }

        bool TCPSession::isConnected()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (active_connection_)
            {
                return active_connection_->IsConnected();
            }
            return false;
        }

        void TCPSession::start()
        {
            client_.Connect();
        }

        void TCPSession::stop()
        {
            client_.Disconnect();
        }

        void TCPSession::setOnFunctionCallback(std::function<void(const evpp::TCPConnPtr & conn, evpp::Buffer * buf)> onMessageFunction)
        {
            this->onMessageFunction = onMessageFunction;
        }

        /* This function is called whenever the client establishes a connection to the server */
        void TCPSession::onConnectionEstablished(const evpp::TCPConnPtr &conn)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (conn->IsConnected())
            {
                active_connection_ = conn;
            }
            else
            {
                active_connection_.reset();
            }
        }
        //Review this 
        void TCPSession::defaultOnMessageFromServer(const evpp::TCPConnPtr& conn, evpp::Buffer* buf)
        {
            bool byPass = hasHeader;

            while (buf->size() > 0)
            {
                //std::cout << "Server received something " << tempId << "Server read" << buf->size() << std::endl;
                if (!hasHeader)//is used for checking if we are collecting data, or are we collecting the header 
                {
                    if (header_buffer.size() < connectionData->headerSize)
                    {
                        header_buffer.Append(buf->Next(connectionData->headerSize - header_buffer.size()));
                    }

                    if (header_buffer.size() == sizeof(Trials::DataHeader))
                    {
                        struct Trials::DataHeader* incoming_header = (Trials::DataHeader*)header_buffer.data();
                        if (incoming_header != nullptr)
                        {
                            switch (incoming_header->type_)
                            {
                            case 'B':
                                connectionData->headerSize = sizeof(FaceHeader);
                                break;
                            case 'D':
                                connectionData->hasHeader = true;
                                connectionData->bytesReceived = 0;
                                break;
                            }
                        }
                        else
                        {
                            connectionData->hasHeader = false;
                            connectionData->bytesReceived = 0;
                            connectionData->headerSize = sizeof(DataHeader);
                            header_buffer.Reset();
                        }
                    }

                    if (header_buffer.size() == connectionData->headerSize)
                    {
                        connectionData->hasHeader = true;
                        connectionData->bytesReceived = 0;
                    }
                }

                if (hasHeader)//now we have the full header
                {
                    try
                    {
                        struct Trials::DataHeader* incoming_header = (Trials::DataHeader*)header_buffer.data();
                        if (incoming_header != nullptr)
                        {
                            //TODO: Split this into handle packet read, and handle packet complete
                            switch (incoming_header->type_)
                            {
                            case 'B':
                                handleFacePacket(conn, buf, (Trials::FaceHeader*)incoming_header);
                                break;
                            }

                            if (connectionData->bytesReceived == incoming_header->dataLength_)
                            {
                                connectionData->hasHeader = false;
                                connectionData->bytesReceived = 0;
                                connectionData->headerSize = sizeof(DataHeader);
                                header_buffer.Reset();
                            }
                        }
                        else
                        {
                            std::cout << "[TCPSession] Unable to read header due to typecase";
                            hasHeader = false;
                            connectionData->bytesReceived = 0;
                            connectionData->headerSize = sizeof(DataHeader);
                            header_buffer.Reset();
                        }
                    }
                    catch (std::exception e)
                    {
                        std::cout << "[TCPSession] Unable to read header due to e" << e.what();
                        hasHeader = false;
                        connectionData->bytesReceived = 0;
                        connectionData->headerSize = sizeof(DataHeader);
                        header_buffer.Reset();
                    }
                }
            }
            /*
            while (buf->size() >= sizeof(Trials::DataHeader))
            {
                if (!hasHeader)//is used for checking if we are collecting data, or are we collecting the header 
                {
                    if (header_buffer.size() < sizeof(Trials::DataHeader))
                    {
                        //find how far we are from a dataheader, then grab the slice of that from buf and add it to header 
                        header_buffer.Append(buf->Next(sizeof(Trials::DataHeader) - header_buffer.size()));
                    }

                    if (header_buffer.size() == sizeof(Trials::DataHeader))
                    {
                        hasHeader = true;
                        //the header is in network, time to convert to host
                        //Well if things break we can run the test here
                        try
                        {
                            //struct Trials::DataHeader* incoming_header = dynamic_cast<Trials::DataHeader&>(header_buffer.data);
                            struct Trials::DataHeader* incoming_header = (Trials::DataHeader*)header_buffer.data();
                            if (incoming_header != nullptr)
                            {
                                //Congrats successful packet transfer
                                std::cout << "The type is " << incoming_header->type_ << " id:" << conn->id();
                            }
                            else
                            {
                                std::cout << "[TCPSession] Unable to read header";
                            }
                        }
                        catch (std::exception e)
                        {
                            std::cout << "[TCPSession] Unable to read header";
                        }
                        setPacketPending(false);
                    }
                }

                if (hasHeader)
                {
                    //now we looking for data
                    //TODO let server talk with client
                }
            }*/
            
        }

        void TCPSession::onMessageFromServer(const evpp::TCPConnPtr& conn, evpp::Buffer* buf)
        {
            if (this->onMessageFunction)
            {
                this->onMessageFunction(conn, buf);
            }
            else
            {
                this->defaultOnMessageFromServer(conn, buf);
            }
        }

        void TCPSession::handleFacePacket(const evpp::TCPConnPtr& conn, evpp::Buffer* buf, Trials::FaceHeader* headers)
        {
            //Now we can receive the data
            //At this point I have receive the header data
            
            if (connectionData->bytesReceived < headers->dataLength_) //if the pool return an image buffer now we can read data
            {
                evpp::Slice readSlice = buf->Next(headers->dataLength_ - connectionData->bytesReceived);
                unsigned int bytes_read = readSlice.size();

                //I can't remove this memcopy I don't think 
                std::memcpy((unsigned char*)((unsigned char*)data_buffer.data() + connectionData->bytesReceived), (unsigned char*)readSlice.data(), readSlice.size());
                connectionData->bytesReceived += bytes_read;
                
            }

            if (connectionData->bytesReceived == headers->dataLength_)
            {
                std::cout << "Server Sent: " << data_buffer.ToString();
            }
        }
}