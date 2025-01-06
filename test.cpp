#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>

// Initialize SDL_ttf
int main(int argc, char* argv[]){
    std::cout << "Font loading" << std::endl;
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf Initialization failed: " << TTF_GetError() << std::endl;
        exit(1);
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("src/arial.ttf", 24); // 24 is the font size
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        exit(1);
    }
    std::cout << "Font loaded successfully" << std::endl;
    return 0;
}

