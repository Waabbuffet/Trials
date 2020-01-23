#pragma once

#ifndef Main00_H
#define Main00_H

//This header is just meant to hold all of the constants for the different compile options 
#define H_SERVER
#define H_OS_WINDOWS

#ifdef H_OS_WINDOWS
#include <WinSock2.h>
#endif // Enable windows sockets, add else for linux sockets

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200

#define GOOGLE_STRIP_LOG -1

void TestTheTester(int&, int);
#endif // !Main00_H

