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
    class TCPSession : std::enable_shared_from_this<TCPSession>
    {

    private:
        evpp::TCPClient client_; //The evpp tcp client

        Client* owner_;         //The trials client owner
        std::mutex mutex_;      //The mutex locking the active_connection. Prevents changing the active_connection while we are writing to it 
        evpp::TCPConnPtr active_connection_; //The active connection, equivalent to a event loop and a socket
        std::function<void(const evpp::TCPConnPtr & conn, evpp::Buffer * buf)> onMessageFunction;
    public:
        TCPSession(evpp::EventLoop* loop,
            const std::string& serverAddr,
            const std::string& name,
            const int sessionId,
            Client* master_client)
            : client_(loop, serverAddr, name),
            owner_(master_client),
            active_connection_()
        { 
            client_.SetConnectionCallback(
                std::bind(&TCPSession::onConnectionEstablished, this, std::placeholders::_1));
            
            client_.SetMessageCallback(
                std::bind(&TCPSession::onMessageFromServer, this, std::placeholders::_1, std::placeholders::_2));
            client_.set_connecting_timeout(evpp::Duration(10.0));
            
        }

        ~TCPSession()
        {

        }
    public:

        //This function will add a header to the buffet before sending the contents to the user
        void writeToConnection()
        {
            
        }

        //This function will use the session tcp socket to send the contents of buf over to the server
        //Note raw means it will not do any preprocessing on the buffer before the contents are sent over 
        void rawWriteToConnection(evpp::Buffer* buf)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (active_connection_->IsConnected)
            {
                active_connection_->Send(buf);
            }
        }

        void start()  
        {
            client_.Connect();
        }

        void stop() 
        {
            client_.Disconnect();
        }

        void setOnFunctionCallback(std::function<void(const evpp::TCPConnPtr & conn, evpp::Buffer * buf)> onMessageFunction)
        {
            this->onMessageFunction = onMessageFunction;
        }

    private:
        /* This function is called whenever the client establishes a connection to the server */
        void onConnectionEstablished(const evpp::TCPConnPtr &conn)
        {
            //check to see if the connection is established 
            //if it is this is the new connection to the server
            std::lock_guard<std::mutex> lock(mutex_);
            if (conn->IsConnected)
            {
                active_connection_ = conn;
            }
            else
            {
                active_connection_.reset();
            }
        }
        
        void onMessageFromServer(const evpp::TCPConnPtr& conn, evpp::Buffer* buf)
        {
            //Check the function, if no function revert back to the client callback function
        }
    };
}