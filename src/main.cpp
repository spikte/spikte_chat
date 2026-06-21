#include "../include/run_client.hpp"
#include "../include/run_server.hpp"
#include <iostream>

#define RAYGUI_IMPLEMENTATION
#include "../lib/raygui.h"

constexpr int screenWidth = 1920 * 0.4;
constexpr int screenHeight = 1080 * 0.8;

int main(int argc, char *argv[]) {
    if(argc < 2)
        runClient(screenWidth, screenHeight, argv);
    std::string mode = argv[1];
    if(mode == "server")
        runServer(argc, argv);
    else if(mode == "client") {
        runClient(screenWidth, screenHeight, argv);
    } else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        return 1;
    }
    return 0;
}
