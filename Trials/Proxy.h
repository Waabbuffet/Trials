#pragma once
#ifndef Proxy_00
#define Proxy_00

#include "Main.h"

#include <stdio.h>
#include <stdlib.h>

#include <evpp/event_loop.h>
#include <evpp/event_loop_thread.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>


namespace Trials
{
	void handleHTTPReponse();

	void sendHTTPGetRequest();

	void sendHTTPPutRequest();

	void sendHTTPPostRequest();
}



#endif // !Proxy_00
