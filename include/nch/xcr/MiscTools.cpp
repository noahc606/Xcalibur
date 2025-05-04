#include "MiscTools.h"
#include <nch/cpp-utils/shell.h>
#include <nch/cpp-utils/string-utils.h>
#include <SDL2/SDL_image.h>
#include "Xcalibur.h"

using namespace nch;

std::string MiscTools::sdlSurfOCR(SDL_Surface* surf, std::string extraArgs)
{
    //Save image, use Tesseract OCR on file, return result.
    IMG_SavePNG(surf, "temp_ocr_screencap.png");
    std::string ocr;
    if(extraArgs=="") { ocr = Shell::exec("tesseract temp_ocr_screencap.png stdout"); }
    else              { ocr = Shell::exec("tesseract temp_ocr_screencap.png stdout "+extraArgs); }
    return ocr;
}

std::string MiscTools::sdlSurfOCR(SDL_Surface* surf) {
    return sdlSurfOCR(surf, "");
}

std::string MiscTools::sdlTexOCR(SDL_Texture* tex, SDL_Renderer* rend)
{
    std::string res;
    SDL_Texture* target = SDL_GetRenderTarget(rend); {
        SDL_SetRenderTarget(rend, tex);
        int w, h;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Surface* surf = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
        SDL_RenderReadPixels(rend, NULL, surf->format->format, surf->pixels, surf->pitch);
        res = sdlSurfOCR(surf);
        SDL_FreeSurface(surf);
    } SDL_SetRenderTarget(rend, target);

    return res;
}

std::string MiscTools::displayOCR(const Rect& area, std::string extraArgs)
{
    auto surf = Xcalibur::displayToSDLSurf(area);
    std::string ocr = StringUtils::trimmed(MiscTools::sdlSurfOCR(surf, extraArgs));
    SDL_FreeSurface(surf);
    return ocr;
}
std::string MiscTools::displayOCR(const Rect& area) {
    return displayOCR(area, "");
}