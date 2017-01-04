#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

class Character {

    SDL_Rect frame;
    SDL_Texture* texture;
    bool hidden = false;

    public:
        bool up, left, right, down;

        Character():texture(NULL), up(false), left(false), right(false), down(false){}
        ~Character() {
            SDL_DestroyTexture(texture);
        }

        void setFrame(int x, int y, int width, int height) {
            frame = {x,y,width,height};
        }

        void updatePosition(int x, int y) {
            frame.x = x;
            frame.y = y;
        }

        void moveXBy(int n) {
            frame.x += n;
        }

        void moveYBy(int n) {
            frame.y += n;
        }

        void setImage(SDL_Renderer* ren, std::string name) {
            SDL_Surface* surface = IMG_Load(name.c_str());
            texture = SDL_CreateTextureFromSurface(ren, surface);
            SDL_FreeSurface(surface);
        }

        void setHidden(bool b) {
            hidden = b;
        }

        bool isHidden() {
            return hidden;
        }

        SDL_Rect* getFrame() {
            return &frame;
        }

        SDL_Texture* getTexture() {
            return texture;
        }
};

#endif
