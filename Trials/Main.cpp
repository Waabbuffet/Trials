﻿#include "Main.h" 
#include "Client.h"
#include "Server.h"
#include "DataHeader.h"
#include "opencv2/opencv.hpp"
#include "Framebuffer.h"

#include "firebase/app.h"
#include "firebase/database.h"

#include <thread>

#include <stdio.h>
#include <stdlib.h>

#include <evpp/event_loop_thread.h>

#include <evpp/httpc/request.h>
#include <evpp/httpc/conn.h>
#include <evpp/httpc/response.h>
#include <evpp/httpc/ssl.h>
#include <openssl/err.h>

#include "HeapObjectPool.h"

void testOpenCL()
{
    /*
    const int elements = 2048;
    size_t datasize = sizeof(int) * 2048;

    int* A = new int[2048];
    int* B = new int[2048];
    int* C = new int[2048];

    for (int i = 0; i < elements; i++)
    {
        A[i] = i;
        B[i] = i;
    }

    try
    {  // Query for platforms
        std::vector <cl::Platform > platforms;
        cl::Platform::get(&platforms);
        // Get a l i s t of devices on this platform 28
        std::vector <cl::Device> devices;
        platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &devices);

        // Create a context for the devices
        cl::Context context(devices);

        // Create a command−queue for the f i r s t device
        cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);

        // Create the memory buffer

        cl::Buffer bufferA = cl::Buffer(context, CL_MEM_READ_ONLY, datasize);

        cl::Buffer bufferB = cl::Buffer(context, CL_MEM_READ_ONLY, datasize);

        cl::Buffer bufferC = cl::Buffer(context, CL_MEM_WRITE_ONLY, datasize);

        // Copy the input data to the input buffers using th
        // command−queue for the f i r s t device
        queue.enqueueWriteBuffer(bufferA, CL_TRUE, 0, datasize, A);
        queue.enqueueWriteBuffer(bufferB, CL_TRUE, 0, datasize, B);

        // Read the program source
        std::ifstream sourceFile("vecadd.cl");

        std::string sourceCode(std::istreambuf_iterator < char >(sourceFile), (std::istreambuf_iterator < char >()));

        cl::Program::Sources source(1, sourceCode);

        // Create the program from the source cod
        cl::Program program = cl::Program(context, source);

        // Build the program for the devices
        program.build(devices);

        // Create the kernel
        cl::Kernel vecadd_kernel(program, "vecadd");

        // Set the kernel arguments
        vecadd_kernel.setArg(0, bufferA);
        vecadd_kernel.setArg(1, bufferB);
        vecadd_kernel.setArg(2, bufferC);

        cl::NDRange global(elements);
        cl::NDRange local(256);
        queue.enqueueNDRangeKernel(vecadd_kernel, cl::NullRange, global, local);

        queue.enqueueReadBuffer(bufferC, CL_TRUE, 0, datasize, C);

    }
    catch (cl::Error error)
    {
        std::cout << error.what() << "(" << error.err() << ")" << std::endl;
    }*/
}

//Init pool and frame buffer
Trials::HeapObjectPool<cv::Mat, MAX_FRAME_BUFFER> Trials::Server::pool;
Trials::FrameBuffer Trials::Server::frame(MAX_FRAME_BUFFER, WAIT_FOR_X_FRAMES, TIME_BETWEEN_FRAMES_MS);

/*
int main(int, char**)
{
    try
    {
        // That's all that is needed to do cleanup of used resources (RAII style).
        curlpp::Cleanup myCleanup;

        // Our request to be sent.
        curlpp::Easy myRequest;

        // Set the URL.
        myRequest.setOpt<Url>("https://nmk01-5be95.firebaseio.com/friendlist.json");

        // Send request and get a result.
        // By default the result goes to standard output.
        myRequest.perform();
    }

    catch (curlpp::RuntimeError & e)
    {
        std::cout << e.what() << std::endl;
    }

    catch (curlpp::LogicError & e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}*/

int main(int argc, char* argv[])
{
#ifdef H_OS_WINDOWS
    WSADATA wsaData;

    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
#endif // DEBUG

    /* Here is how the client will work, easy prototype
       1. Create N image packets for the client
       2. Init the client with X sessions
       3. Create a polling loop, if the client is ready and has no frame in transaction
       4. Send the frame using the session
       5. Wait for response
       6. Handle response
       7. Repeat loop
    */
#ifndef H_SERVER
    int numberSessions = 1;
    int numberThreads = 1;

    cv::VideoCapture cap(0);
    if (!cap.isOpened())
        return -1;

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);


    Trials::HeapObjectPool<cv::Mat, 128> thepool;
    Trials::OpenCVMatrixRequirement require_(640, 480, 3, CV_8UC3);

    evpp::EventLoopThread loop;
    std::string serverAddr = "10.0.0.192:9099";

    struct Trials::ImageHeader data_(640, 480, 1, false, 1);
    evpp::Buffer the_buffer(sizeof(Trials::DataHeader));
    Trials::Client client(loop.loop(), "trials_client", serverAddr, 1, 1);

    loop.Start(true);
    cv::Mat* clientBuffers[1];
    clientBuffers[0] = pool.acquire(&require_);
    std::vector<uchar> encodedFrame;

   while(true)
   {
      //  if (!client.isSessionWaiting(0))
        {
            if (!client.isSessionConnected(0))
            {
                client.waitForSessionConnection(0);
            }

            *clientBuffers[0] = cv::imread("C:\\Users\\Waabbuffet\\Desktop\\COE848-Lab2\\Untitled3.png");
            //cap >> *clientBuffers[0];
            cv::imencode(".jpeg", *clientBuffers[0], encodedFrame);
            //data_.dataLength_ = clientBuffers[0]->total() * clientBuffers[0]->elemSize();
            data_.dataLength_ = encodedFrame.size();
            data_.isEncoded_ = true;

            the_buffer.Write(&data_, sizeof(data_));
            the_buffer.Write(encodedFrame.data(), data_.dataLength_);
            //std::cout << "Dylan you son of a " << the_buffer.size();

            client.writeToServer(0, &the_buffer);
            the_buffer.Reset();
            //std::cout << "the data is" << data_.dataLength_ << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    std::string line;
    while (std::getline(std::cin, line))
    {
        if (line == "quit") {
            client.stopAll();
            break;
        }
    }

    // if (!client.isSessionConnected(0))
    //     client.waitForSessionConnection(0);

    //client.writeToServer(0, &the_buffer);

    loop.Stop(true);
#else
   std::thread thread_object(&Trials::FrameBuffer::run, &Trials::Server::frame);

   evpp::EventLoop loop; //changed from event loop

   std::string serverAddr = std::string(SERVER_NETWORK_IP) + ":" + SERVER_NETWORK_PORT;
   Trials::Server server(&loop, serverAddr, "trials_server", 1);
 

   server.Start();
   loop.Run();
#endif // DEBUG
    return 0;
}
void TestTheTester(int& a, int b)
{
    if (b < a)
    {
        a = b;
    }
}