#include "Xcalibur.h"
#include <nch/cpp-utils/log.h>
#include <nch/cpp-utils/timer.h>
#include <nch/sdl-utils/texture-utils.h>
#include <nch/xcr/MiscTools.h>

using namespace nch;

bool Xcalibur::initted = false;
SDL_Texture* Xcalibur::screenTex = nullptr;
SDL_Surface* Xcalibur::screenSurf = nullptr;
SDL_Renderer* Xcalibur::rend = nullptr;

XImage* Xcalibur::ximg = nullptr;
Display* Xcalibur::disp = nullptr;
XShmSegmentInfo Xcalibur::shmSegInfo;
Rect Xcalibur::dispArea;

std::set<std::pair<int, int>> Xcalibur::pixSet;
std::map<std::pair<int, int>, nch::Color> Xcalibur::pixDiffs;
std::vector<nch::Rect> Xcalibur::ignoredPixAreas;

void Xcalibur::init(SDL_Renderer* rend, const Rect& displayArea)
{
    /* Init members */
    if(initted) {
        Log::error(__PRETTY_FUNCTION__, "Already initialized.");
        return;
    }
    Xcalibur::rend = rend;

    /* Open clipboard */
    MiscTools::globalInitLibclipboard();

    /* Open/Setup X11 display */
    {
        //Open display
        disp = XOpenDisplay(NULL);
        if(disp==NULL) {
            printf("Failed to open display...\n");
        }
        //Print info...
        int snum = DefaultScreen(disp);
        int dWidth = DisplayWidth(disp, snum);
        int dHeight = DisplayHeight(disp, snum);
        Log::log("Using display with dimensions %dx%d", dWidth, dHeight);
        //Set 'dispArea'
        if(displayArea==Rect::createFromTwoPts(0, 0, -1, -1)) {
            Xcalibur::dispArea = Rect(0, 0, dWidth, dHeight);
        } else {
            //Clip dispArea if necessary
            Rect cDispArea = displayArea;
            if(displayArea.x1()<0)          cDispArea.r.x = 0;
            if(displayArea.y1()<0)          cDispArea.r.y = 0;
            if(displayArea.x2()>dWidth)     cDispArea.r.w = dWidth-cDispArea.r.x;
            if(displayArea.y2()>dHeight)    cDispArea.r.h = dHeight-cDispArea.r.y;
            if(cDispArea!=displayArea) {
                Log::warnv(__PRETTY_FUNCTION__, "clipping 'dispArea'", "Bounds of 'dispArea' exceeds the screen's bounds");
            }

            Xcalibur::dispArea = cDispArea;
        }


    }

    /* Create X shared memory image */
    {
        //Create x11 image
        ximg = XShmCreateImage(disp, DefaultVisual(disp, 0), 24, ZPixmap, NULL, &shmSegInfo, dispArea.r.w, dispArea.r.h);
        //Set properties of 'shmSegInfo' from the new 'ximg'.
        shmSegInfo.shmid = shmget(IPC_PRIVATE, ximg->bytes_per_line*ximg->height, IPC_CREAT|0777);
        shmSegInfo.shmaddr = ximg->data = (char*)shmat(shmSegInfo.shmid, 0, 0);
        shmSegInfo.readOnly = False;
        //Attach to 'disp'lay
        if(XShmAttach(disp, &shmSegInfo)==0) {
            Log::errorv(__PRETTY_FUNCTION__, "XShmAttach()", "function returned error code of 0");
            return;
        }
    }

    /* Create SDL Texture to stream to */
    screenTex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, dispArea.r.w, dispArea.r.h);
    if(screenTex==NULL) {
        Log::errorv(__PRETTY_FUNCTION__, "SDL_CreateTexture()", SDL_GetError());
        return;
    }
    screenSurf = SDL_CreateRGBSurfaceWithFormat(0, dispArea.r.w, dispArea.r.h, 32, SDL_PIXELFORMAT_BGRA32);
    if(screenSurf==NULL) {
        Log::errorv(__PRETTY_FUNCTION__, "SDL_CreateRGBSurfaceWithFormat()", SDL_GetError());
        return;
    }
    for(int ix = 0; ix<screenSurf->w; ix++)
    for(int iy = 0; iy<screenSurf->h; iy++) {
        TexUtils::setPixelColor(screenSurf, ix, iy, Color(255, 0, 0).getRGBA());
    }

    /* Valid init by this point */
    initted = true;

    /* Pixel set */
    resetPixSet();
}

void Xcalibur::free()
{
    if(!initted) {
        Log::error(__PRETTY_FUNCTION__, "Already freed.");
        return;
    }

    /* Close clipboard */
    MiscTools::globalFreeLibclipboard();

    /* Pointers */
    SDL_DestroyTexture(screenTex);  //Destroy screen texture
    SDL_FreeSurface(screenSurf);    //Destroy previous screen surface
    XDestroyImage(ximg);            //Destroy image
    XShmDetach(disp, &shmSegInfo);  //Shared memory detatch from display
    XCloseDisplay(disp);            //Destroy display

    /* Other states */
    pixSet.clear();
    ignoredPixAreas.clear();
    initted = false;
}

void Xcalibur::updatePixDiffs()
{
    if(!initted) {
        Log::error(__PRETTY_FUNCTION__, "Xcalibur is not initialized (Xcalibur::init)");
        return;   
    }

    Xcalibur::streamScreen();

    pixDiffs.clear();
    for(auto pixLoc : pixSet) {
        Color px0 = TexUtils::getPixelColor(screenSurf, pixLoc.first, pixLoc.second);
        Color px1 = TexUtils::getPixelColor((void*)ximg->data, screenSurf->format, ximg->bytes_per_line, pixLoc.first, pixLoc.second);
        
        if(pixLoc==std::make_pair(0, 0)) {
            //printf("breakpoint\n");
        }

        if(px0!=px1) {
            pixDiffs.insert({{pixLoc.first, pixLoc.second}, px1});
        }
    }
}
void Xcalibur::resetScreenSurf()
{
    if(!initted) {
        Log::error(__PRETTY_FUNCTION__, "Xcalibur is not initialized (Xcalibur::init)");
        return;   
    }

    SDL_FillRect(screenSurf, NULL, SDL_MapRGB(screenSurf->format, 0, 0, 0));
}
void Xcalibur::updateScreenSurf()
{
    if(!initted) {
        Log::error(__PRETTY_FUNCTION__, "Xcalibur is not initialized (Xcalibur::init)");
        return;   
    }

    Xcalibur::streamScreen();

    //Copy pixels from X display to SDL_Surface
    uint8_t* srcPixels = static_cast<uint8_t*>((void*)ximg->data);
    uint8_t* dstPixels = static_cast<uint8_t*>(screenSurf->pixels);
    for(int row = 0; row<screenSurf->h; row++) {
        memcpy(dstPixels + row*screenSurf->pitch, srcPixels + row*ximg->bytes_per_line, screenSurf->w*screenSurf->format->BytesPerPixel);
    }
}
void Xcalibur::streamScreen()
{
    if(!initted) {
        Log::error(__PRETTY_FUNCTION__, "Xcalibur is not initialized (Xcalibur::init)");
        return;   
    }

    //Get ximg data from screen
    XShmGetImage(disp, RootWindow(disp, 0), ximg, dispArea.r.x, dispArea.r.y, AllPlanes);
    //Set pixels within 'surf' with data from 'ximg'
    SDL_UpdateTexture(screenTex, NULL, ximg->data, ximg->bytes_per_line);
}

SDL_Surface* Xcalibur::displayToSDLSurf(const nch::Rect& area)
{
    if(!initted) {
        Log::error(__PRETTY_FUNCTION__, "Xcalibur is not initialized (Xcalibur::init)");
        return nullptr;   
    }

    updateScreenSurf();
    
    //Create surface
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, area.r.w, area.r.h, 32, SDL_PIXELFORMAT_ABGR32);
    //Populate surface pixels
    for(int ix = 0; ix<area.r.w; ix++)
    for(int iy = 0; iy<area.r.h; iy++) {
        Color c = TexUtils::getPixelColor(screenSurf, area.r.x+ix, area.r.y+iy); c.a = 255;
        TexUtils::setPixelColor(surf, ix, iy, c.getRGBA());
    }
    //Return surface
    return surf;
}
nch::Color Xcalibur::getDisplayPixelColor(int x, int y) {
    if(!initted) {
        Log::error(__PRETTY_FUNCTION__, "Xcalibur is not initialized (Xcalibur::init)");
        return nch::Color();   
    }
    
    if(x<0 || x>=screenSurf->w || y<0 || y>=screenSurf->h)
        return Color(0, 0, 0, 0);
    Color c = TexUtils::getPixelColor(screenSurf, x, y); c.a = 255;
    return c;
}
bool Xcalibur::checkDisplayPixels(const std::vector<nch::Vec2i>& pixCoords, const nch::Color& pixColor)
{
    if(!initted) {
        Log::error(__PRETTY_FUNCTION__, "Xcalibur is not initialized (Xcalibur::init)");
        return false;
    }

    for(int i = 0; i<pixCoords.size(); i++) {
        Color c = getDisplayPixelColor(pixCoords[i].x, pixCoords[i].y);
        if(c!=pixColor)
            return false;
    }

    return true;
}
SDL_Texture* Xcalibur::getCapturedScreenTex() {
    return screenTex;
}
SDL_Surface* Xcalibur::getCapturedScreenSurf() {
    return screenSurf;
}
Rect Xcalibur::getCapturedScreenRect() {
    return dispArea;
}
std::vector<nch::Rect> Xcalibur::getIgnoredPixAreas() {
    return ignoredPixAreas;
}
std::map<std::pair<int, int>, nch::Color> Xcalibur::getPixDiffs() {
    return pixDiffs;
}

void Xcalibur::addIgnoredPixSet(const nch::Rect& r) {
    if(!initted) {
        Log::error(__PRETTY_FUNCTION__, "Xcalibur is not initialized (Xcalibur::init)");
        return;
    }
    
    ignoredPixAreas.push_back(r);
    for(int ix = r.x1(); ix<=r.x2(); ix++)
    for(int iy = r.y1(); iy<=r.y2(); iy++) {
        auto f = pixSet.find(std::make_pair(ix, iy));
        if(f!=pixSet.end()) {
            pixSet.erase(f);
        }
    }
}
void Xcalibur::resetPixSet() {
    if(!initted) {
        Log::error(__PRETTY_FUNCTION__, "Xcalibur is not initialized (Xcalibur::init)");
        return;
    }

    pixSet.clear();
    ignoredPixAreas.clear();
    for(int ix = 0; ix<dispArea.r.w; ix++)
    for(int iy = 0; iy<dispArea.r.h; iy++) {
        pixSet.insert(std::make_pair(ix, iy));
    }
}
