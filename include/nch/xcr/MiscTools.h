#pragma once
#include <SDL2/SDL.h>
#include <nch/sdl-utils/rect.h>
#include <string>

namespace nch { class MiscTools {
public:
    /// @brief Quickly save a SDL_Surface* to disk, perform Tesseract OCR on it, and return the result. You may want to trim the result using StringUtils::trimmed().
    /// @param surf The SDL_Surface* object to perform OCR on.
    /// @param extraArgs If not "": extra arguments to use for the 'tesseract' command (ex: "--psm 6").
    /// @return The OCR-detected text found within 'surf', as a single string (may include newlines).
    static std::string sdlSurfOCR(SDL_Surface* surf, std::string extraArgs);
    static std::string sdlSurfOCR(SDL_Surface* surf);
    /// @brief Same as 'sdlSurfOCR' but with an SDL_Texture*.
    /// @param tex The SDL_Texture* object to perform OCR on.
    /// @param rend The SDL_Renderer* object to use 'SDL_RenderReadPixels()' with (render target should be restored)
    /// @return The OCR-detected text found within 'tex', as a single string (may include newlines).
    static std::string sdlTexOCR(SDL_Texture* tex, SDL_Renderer* rend);

    /// @brief Perform OCR on the Xcalibur display, in a similar manner to 'sdlSurfOCR'.
    /// @param area The area of the display to perform OCR on.
    /// @param extraArgs See extraArgs from 'sdlSurfOCR'
    /// @return The OCR-detected text found within the specified 'area' of the Xcalibur display.
    static std::string displayOCR(const Rect& area, std::string extraArgs);
    static std::string displayOCR(const Rect& area);
private:

}; }