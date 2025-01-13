#include "Application.h"
#include <SDL.h>
#include <iostream>
#include <string>

std::ofstream logFile;

// 初始化日志文件
void initLog() {
    logFile.open("debug_log.txt", std::ios::out | std::ios::trunc);
    if (!logFile) {
        std::cerr << "Failed to open log file!" << std::endl;
        exit(1);
    }
    logFile << "Log initialized." << std::endl;
}

// 写入日志
void log(const std::string& message) {
    if (logFile.is_open()) {
        logFile << message << std::endl;
    }
}

// 在程序结束时关闭日志
void closeLog() {
    if (logFile.is_open()) {
        logFile << "Log closed." << std::endl;
        logFile.close();
    }
}

Application::Application() {
    initLog(); // 初始化日志
    log("Application starting...");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        logFile << "SDL Initialization failed: " << SDL_GetError() << std::endl;
        exit(1);
    }
    window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    menu = new Menu(renderer);
    game = new Game(renderer);
}

Application::~Application() {
    delete menu;
    delete game;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Application::run() {
    bool running = true;

    while (running) {
        menu->show();
        if (menu->startSelected()) {
            game->reset();
            game->show();
            menu->resetState();
        } else {
            running = false;
        }
    }
}