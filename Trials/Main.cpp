#include "Main.h" //if this appears are error ignore this 

#include <fstream>
#include <string>
#include <iostream>

#ifdef H_OS_WINDOWS
#include <winsock2.h>
#endif

#include <ws2tcpip.h>
#include <stdio.h>
#include <evpp/tcp_server.h>
#include <evpp/tcp_client.h>
#include <evpp/buffer.h>
#include <evpp/tcp_conn.h>

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




    
    return 0;
}


void TestTheTester(int &a, int b)
{
	if (b < a)
	{
		a = b;
	}
}
