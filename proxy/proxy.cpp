#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <fstream>
#include "enet/include/enet.h"
#include "http.h"
#include "server.h"
server* g_server = new server();
#include "json.hpp"

using json = nlohmann::json;

int main() {
#ifdef _WIN32
    SetConsoleTitleA("proxy by ama & modified by Viam#1337");
#endif
    printf("enet proxy by ama & modified by Viam#1337\n");

	
	std::ifstream configs("config.json");
	if (!configs.is_open()) {
		printf("failed to load config.json!\n");
		Sleep(3000);
		exit(0);
	}
	json js;
	configs >> js;
	configs.close();
	std::string ip = js["local_ip"];
	
    std::thread http(http::run, ip, "1");
    http.detach();
    printf("HTTP server is running.\n");

    enet_initialize();
    if (g_server->start()) {
        printf("Server & client proxy is running.\n");
        while (true) {
            g_server->poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }   
    else
        printf("Failed to start server or proxy.\n");
}
