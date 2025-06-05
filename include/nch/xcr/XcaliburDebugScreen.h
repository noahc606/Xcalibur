#pragma once
#include <SDL2/SDL.h>
#include <nch/sdl-utils/rect.h>
#include <nch/sdl-utils/text.h>
#include <vector>

namespace nch { class XcaliburDebugScreen {
public:
    void free();
    
    void draw(SDL_Renderer* rend);
    void setRenderTargetHere(SDL_Renderer* rend);


    std::vector<Rect> indicators;  
private:
    int lastW = -1; int lastH = -1;
    SDL_Texture* dbOverlay = nullptr;
    nch::Text dbInfo;
}; }