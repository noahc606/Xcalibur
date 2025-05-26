#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <nch/xcr/XcaliburDebugScreen.h>

class Main {
public:
    Main();
    ~Main();

    static SDL_Renderer* getRenderer();
private:
    static void tick();
    static void draw();
    
    bool tests();

    static SDL_PixelFormat* pxFmt;
    static SDL_Renderer* rend;
    static SDL_Window* win;
    static std::string basePath;
    static nch::XcaliburDebugScreen dbscr;
    static int numTicks;
};