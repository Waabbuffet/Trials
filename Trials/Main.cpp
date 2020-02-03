#include "Main.h" 
#include "Client.h"
#include "Server.h"
#include "DataHeader.h"
#include "opencv2/opencv.hpp"
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

    //Trials::HeapObjectPool<cv::Mat, 128> pool = Trials::HeapObjectPool<cv::Mat, 128>();
    //Trials::OpenCVMatrixRequirement require_(10, 10, 10, 'f');
    //cv::Mat* ptr = pool.acquire(&require_);

    //6 220 800 bytes for 1 1920x1080 image
    //1920x1080x3 because of each channel 
    //Image is only 614,400 bytes tho, 
    //how did this happen?
    //We are sending raw matrix, we need to use encodings but I do not know how to get encoded frame
    //Only decoded frame
#ifndef H_SERVER

        int numberSessions = 1;
        int numberThreads = 1;

        cv::VideoCapture cap(0);
        if (!cap.isOpened())
            return -1;

        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1080);

        Trials::HeapObjectPool<cv::Mat, 128> pool = Trials::HeapObjectPool<cv::Mat, 128>();
        Trials::OpenCVMatrixRequirement require_(1920, 1080, 3, CV_8UC3);
    
        evpp::EventLoopThread loop;
        std::string serverAddr = "localhost:9099";

        struct Trials::ImageHeader data_(1920, 1080, 1, false, 1);
        evpp::Buffer the_buffer(sizeof(Trials::DataHeader));
        Trials::Client client(loop.loop(), "trials_client", serverAddr, 1, 1);
   
        loop.Start(true);
        cv::Mat* clientBuffers[1];
        clientBuffers[0] = pool.acquire(&require_);

        for (int i = 0; i < 1; i ++)
        {
            if (!client.isSessionWaiting(i))
            {
                if (!client.isSessionConnected(i))
                {
                    client.waitForSessionConnection(i);
                }

                *clientBuffers[0] = cv::imread("C:\\Users\\Waabbuffet\\Desktop\\COE848-Lab2\\index.jpg");
                //cap >> *clientBuffers[0];
                data_.dataLength_ = clientBuffers[0]->total() * clientBuffers[0]->elemSize();
                
                the_buffer.Write(&data_, sizeof(data_));
                the_buffer.Write(clientBuffers[0]->data, data_.dataLength_);
                client.writeToServer(0, &the_buffer);
                std::cout << "the data is" << data_.dataLength_ << std::endl;
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
    evpp::EventLoop loop;
    std::string serverAddr = std::string("0.0.0.0") +":9099";
    Trials::Server server(&loop, serverAddr, "trials_client" , 1);
    server.Start();
    loop.Run();
#endif // DEBUG
    return 0;
}

void TestTheTester(int &a, int b)
{
	if (b < a)
	{
		a = b;
	}
}
