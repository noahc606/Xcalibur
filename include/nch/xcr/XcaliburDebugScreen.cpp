#include "XcaliburDebugScreen.h"
#include <nch/cpp-utils/log.h>
#include <nch/sdl-utils/texture-utils.h>
#include <nch/xcr/Xcalibur.h>

using namespace nch;

void XcaliburDebugScreen::free() {
    SDL_DestroyTexture(dbOverlay);
}

void XcaliburDebugScreen::draw(SDL_Renderer* rend)
{
    /* Rebuild 'dbOverlay if needed */
    {
        int w, h;
        SDL_QueryTexture(Xcalibur::getCapturedScreenTex(), NULL, NULL, &w, &h);
        if(lastW!=w || lastH!=h) {
            lastW = w; lastH = h;
            
            Log::log("Rebuilt Xcalibur debug screen");
            SDL_DestroyTexture(dbOverlay);
            dbOverlay = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
            SDL_SetTextureBlendMode(dbOverlay, SDL_BLENDMODE_BLEND);
        }
    }

    /* Draw on-screen overlays */
    TexUtils::clearTexture(rend, dbOverlay);
    SDL_SetRenderTarget(rend, dbOverlay);
    {
        //Draw ignored pixel areas onto 'dbOverlay'
        auto iAreas = Xcalibur::getIgnoredPixAreas();
        SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
        for(int i = 0; i<iAreas.size(); i++) {
            SDL_SetRenderDrawColor(rend, 255, 0, 0, 100);
            SDL_RenderFillRect(rend, &iAreas[i].r);
        }
        //Draw green area indicators into 'dbOverlay
        for(int i = 0; i<indicators.size(); i++) {
            SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(rend, 0, 255, 0, 100);
            SDL_RenderFillRect(rend, &indicators[i].r);
        }

    }
    SDL_SetRenderTarget(rend, NULL);

    
    /* Draw Xcalibur debug screen */
    {
        //Draw texture
        SDL_RenderCopy(rend, Xcalibur::getCapturedScreenTex(), NULL, NULL);
        //Draw 'dbOverlay'
        SDL_RenderCopy(rend, dbOverlay, NULL, NULL);
    }
}

void XcaliburDebugScreen::setRenderTargetHere(SDL_Renderer* rend)
{
    SDL_SetRenderTarget(rend, dbOverlay);
}