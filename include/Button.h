#ifndef BUTTON_H
#define BUTTON_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <functional>
#include <string>

class Button {
public:
    Button(SDL_Renderer* renderer, const std::string& text, const SDL_Rect& rect, std::function<void()> onClick);
    ~Button();

    void render(bool isSelected);
    void setOnClick(std::function<void()> onClick);
    void handleClick();
    std::string getText() const;

private:
    SDL_Renderer* renderer;
    SDL_Rect rect;
    std::string text;
    SDL_Texture* textTexture;
    std::function<void()> onClick;
};

#endif // BUTTON_H
