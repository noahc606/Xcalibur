#pragma once
#include <SDL2/SDL.h>
#include <nch/sdl-utils/text.h>

namespace nch { class XcaliburDebugScreen {
public:
    void free();
    
    void draw(SDL_Renderer* rend);

private:
    int lastW = -1; int lastH = -1;
    SDL_Texture* dbOverlay = nullptr;
    nch::Text dbInfo;
}; }