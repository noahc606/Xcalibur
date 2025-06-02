#include "Main.h"
#include <nch/cpp-utils/log.h>
#include <nch/cpp-utils/shell.h>
#include <nch/cpp-utils/string-utils.h>
#include <nch/cpp-utils/timer.h>
#include <nch/sdl-utils/main-loop-driver.h>
#include <nch/sdl-utils/texture-utils.h>
#include <nch/xcr/Xcalibur.h>
#include <nch/xcr/XTools.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/shm.h>

using namespace nch;

SDL_PixelFormat* Main::pxFmt = nullptr;
SDL_Renderer* Main::rend = nullptr;
SDL_Window* Main::win = nullptr;
std::string Main::basePath = "???null???";
XcaliburDebugScreen Main::dbscr;
int Main::numTicks = 0;

int main(int argc, char** args) {
    Main m; return 0;
}
Main::Main()
{
    /* Initialization */ {
        //Init SDL stuff
        SDL_Init(0);
        pxFmt = SDL_AllocFormat(SDL_PIXELFORMAT_RGB24);
        win = SDL_CreateWindow("Xcalibur", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 480, SDL_WINDOW_RESIZABLE);
        rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
        basePath = SDL_GetBasePath();
        //IMG
        IMG_Init(IMG_INIT_PNG);
        //XCR
        Xcalibur::init(rend, Rect(0, 0, 1920, 1080));
        Xcalibur::addIgnoredPixSet(Rect(0, 1080-45, 1920, 45));
    }

    /* Tests */
    bool cont = tests();
    if(!cont) return;

    /* Main loop */
    MainLoopDriver mld(tick, 20, draw, 60);
}

Main::~Main()
{
    //Free Xcalibur
    Xcalibur::free();
    //SDL stuff to cleanup
    SDL_FreeFormat(pxFmt);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

SDL_Renderer* Main::getRenderer() {
    return rend;
}

void Main::tick()
{
    numTicks++;
    if(numTicks%20==0) {
        //Xcalibur::updatePixDiffs();
        auto pos = XTools::getMouseXY();
        Xcalibur::updateScreenSurf();
        Log::log("Color @ (%d, %d)==%s", pos.x, pos.y, Xcalibur::getDisplayPixelColor(pos.x, pos.y).toStringReadable(false).c_str());
    }
}

void Main::draw()
{
    //Clear objects with black color
    SDL_RenderClear(rend);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
    SDL_RenderFillRect(rend, NULL);

    //Stream screen to Xcalibur
    {
        Xcalibur::streamScreen();
    }

    //Debug screen
    dbscr.draw(rend);

    //Keyboard

    //Render all present objects
    SDL_RenderPresent(rend);
}

bool Main::tests()
{
    return true;
}

