#include "Button.h"
#include <iostream>
#include <fstream>

extern std::ofstream logFile;
extern void log(const std::string& message);

Button::Button(SDL_Renderer* renderer, const std::string& text, const SDL_Rect& rect, std::function<void()> onClick)
    : renderer(renderer), rect(rect), text(text), onClick(onClick) {
    TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24);
    SDL_Color textColor = {0, 0, 0, 0};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);
}

Button::~Button() {
    SDL_DestroyTexture(textTexture);
}

void Button::render(bool isSelected) {
    if (isSelected) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Highlighted color
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Default color
    }

    // Button background
    SDL_RenderFillRect(renderer, &rect);

    // Text
    int textWidth, textHeight;
    SDL_QueryTexture(textTexture, nullptr, nullptr, &textWidth, &textHeight);
    SDL_Rect textRect = {rect.x + (rect.w - textWidth) / 2, rect.y + (rect.h - textHeight) / 2, textWidth, textHeight};
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
}

void Button::setOnClick(std::function<void()> onClick) {
    this->onClick = onClick;
}

void Button::handleClick() {
    if (onClick) {
        onClick();
    }
}

std::string Button::getText() const{
    return text;
}