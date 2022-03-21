#include "http.h"
#include <cstring>
#ifdef __linux__
#include <unistd.h>
#endif
#include "server.h"
#ifndef _WIN32
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==NULL
#endif


sb_Options options;
sb_Server* http_server;

std::string ip;
std::string port;


std::string format(const char* msg, ...) {
	char fmt[1024] = { 0 };
	va_list va;
	va_start(va, msg);
	vsnprintf(fmt, 1024, msg, va);
	va_end(va);
	return std::string(fmt);
}

uint8_t* read_file(const char* file, uint32_t* size) {
	FILE* fp;
	fopen_s(&fp, file, "rb");
	if (!fp)
		return 0;
	fseek(fp, 0, SEEK_END);
	uint32_t fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	uint8_t* data = (uint8_t*)(malloc(fsize));
	if (data) {
		memset(data, 0, fsize);
		fread(data, fsize, 1, fp);
		fclose(fp);
		if (size)
			*size = fsize;
		return data;
	}
	return NULL;
}

#define PRINT(msg, ...) printf("[HTTP]: " msg, ##__VA_ARGS__);
using namespace std;



int http::handler(sb_Event* evt)
{
	if (evt->type == SB_EV_REQUEST) {
		/*
		std::string filename = "C:\\Users\\PC\\Desktop\\Dosyalar\\gt\\V-Proxy\\request.py";
		std::string command = "python ";
		command += filename;
		system(command.c_str());
		*/
		//old
		/*
		
			
			sb_writef(evt->stream, format(server_data, ip.c_str(), port.c_str(), ip.c_str()).c_str());



		*/
		string address = evt->address;
		string request = evt->method;

		PRINT("%s - %s %s\n", evt->address, evt->method, evt->path);
			PRINT("got server data request.\n");
			sb_send_status(evt->stream, 200, "OK");
			sb_send_header(evt->stream, "Content-Type", "text/plain");
			
			string req_address = address;
			char reply[1024];
			string info = "server|%s\nport|%s\ntype|1\n#maint|`4Oh no! `oWait 0 seconds before logging in again!\nbeta_server|0.0.0.0\nbeta_port|1945\nbeta_type|1\nmeta|ni.com\nRTENDMARKERBS1001\n";
			memcpy(reply, info.c_str(), info.size() + 1);
			sb_writef(evt->stream, format(reply, ip.c_str(), port.c_str(), ip.c_str()).c_str());


	}
	return SB_RES_OK;
}

void http::start()
{
	options.handler = handler;
	options.host = "0.0.0.0";
#ifdef __linux__
	if (geteuid() != 0)
	{
	PRINT("You are not root user, Running port 8080\n");
	options.port = "8080";
	}
	else
	{
	PRINT("You are root user, Running port 80\n");
	options.port = "80";
	}
#elif _WIN32
	options.port = "80";
#endif
	http_server = sb_new_server(&options);
	if (!http_server) {
		PRINT("failed to start the http server!\n");
	}
}

#ifdef _WIN32
#include <Windows.h>
#endif

void util_sleep(int32_t ms) {
#ifdef _WIN32
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif

}
void http::run(std::string dest, std::string port2) {
	ip = dest;
	port = port2;
	start();
	while (true) {
		sb_poll_server(http_server, 10);
		util_sleep(1);
	}
	sb_close_server(http_server);
}
