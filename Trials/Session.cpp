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
                std::cout << "New connection established! " << this->sessionId << "vs" << conn->id();
                active_connection_ = conn;
            }
            else
            {
                active_connection_.reset();
            }
        }
        
        void TCPSession::defaultOnMessageFromServer(const evpp::TCPConnPtr& conn, evpp::Buffer* buf)
        {
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
            }
            
        }
        
        //This function is called whenever the server responds with a message 
        //If the session has no registered callback function the client 
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
}