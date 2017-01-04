#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include "Character.hpp"

class Window {

    SDL_Window *window;
    SDL_Renderer* screenRenderer;

    public:

        Window() : window(NULL), screenRenderer(NULL) {}
        ~Window() {
            SDL_DestroyRenderer(screenRenderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
        }

        void createWindow(std::string title, int width, int height) {
            // initialize all of the things
            SDL_Init(SDL_INIT_EVERYTHING);
            // create window
            window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
            // initialize the screen renderer. -1 is to say use the first driver that supports the thrid parameter
            screenRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            // initlialize png support
            IMG_Init(IMG_INIT_PNG);
        }

        void setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
            SDL_SetRenderDrawColor(screenRenderer, r, g, b, a);
        }

        void drawObjectToScreen(Character* obj) {
            if (!obj->isHidden()) {
                SDL_RenderCopy(screenRenderer, obj->getTexture(), NULL, obj->getFrame());
            }
        }

        SDL_Renderer* getRenderer() {
            return screenRenderer;
        }
};


#endif
