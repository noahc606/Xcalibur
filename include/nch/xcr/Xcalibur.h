#pragma once
#include <SDL2/SDL.h>
#include <map>
#include <nch/cpp-utils/color.h>
#include <nch/math-utils/vec2.h>
#include <nch/sdl-utils/rect.h>
#include <set>
/**/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/shm.h>
/**/

class Xcalibur {
public:
    /// @brief Initialize the Xcalibur singleton. Allows access to the screen pixels of an X11 display as specified by 'dispArea'.
    /// @param rend The SDL_Renderer* object used to render the screen (or part of it) to an internal SDL_Texture ('screenTex').
    /// @param dispArea The rectangular area of the screen we are rendering, where xy(0, 0) is the top left.
    static void init(SDL_Renderer* rend, const nch::Rect& dispArea);
    /// @brief Free/destroy the Xcalibur singleton. Can be re-'init()'-ted later if needed.
    static void free();
    
    /// @brief Track which pixels have changed from the previous time 'updatePixDiffs()' was called.
    /// @brief If called for the first time, the "previous set" of pixels are considered to be all black (0,0,0,0).
    /// @brief This function is slow (on my hardware: ~115ms to ~1100ms for 1920x1080 depending on number of changes).
    static void updatePixDiffs();
    static void resetScreenSurf();
    /// @brief Update the internal screen surface accessor to reflect the current screen. Do this before calling getDisplayPixelColor() or displayToSDLSurf().
    /// @brief This function is relatively fast compared to 'updatePixDiffs()' (on my hardware: ~5-10ms for 1920x1080).
    static void updateScreenSurf();
    /// @brief Stream the pixels grabbed from the screen to an internal SDL_Texture ('screenTex').
    /// @brief Use 'getScreenTex()' to get this SDL_Texture. On my hardware this function takes ~10ms for a 1920x1080 screen.
    static void streamScreen();

    /// @brief Render part of the screen to an SDL_Surface*. 'updateScreenSurf()' should be called before this function.
    /// @param area Rectangular area of Xcalibur's 'disp'lay that should be rendered to an SDL_Surface*.
    /// @return An SDL_Surface* that looks like the specified screen area. You need to free this yourself (SDL_Freesurface(...)).
    static SDL_Surface* displayToSDLSurf(const nch::Rect& area);
    /// @brief Get the color of a single pixel on the screen.
    /// @brief Uses the screen from the last time 'updateScreenSurf()' was called, NOT the current screen.
    /// @param x x position of the pixel.
    /// @param y y position of the pixel.
    /// @return The color of the specified screen pixel.
    static nch::Color getDisplayPixelColor(int x, int y);
    /// @brief Check to see if the given list of pixels ('pixCoords') match the specified color ('pixColor')
    /// @param pixCoords The list of coordinates to check.
    /// @param pixColor The color that all the pixels from 'pixCoords' should be.
    /// @return True if all pixel colors match 'pixColor', false otherwise.
    static bool checkDisplayPixels(const std::vector<nch::Vec2i>& pixCoords, const nch::Color& pixColor);
    /// @return The SDL_Texture* representing the screen, which is rebuilt upon every 'streamScreen()' call.
    static SDL_Texture* getScreenTex();
    /// @return An SDL_Surface* which is used to calculate pixel changes. Rebuilt upon every 'updatePixDiffs()' call.
    static SDL_Surface* getScreenSurf();
    /// @return The list of areas not updated by 'updatePixDiffs()'.
    static std::vector<nch::Rect> getIgnoredPixAreas();
    /// @return The list of pixels that have changed between the 2nd last call and last call of 'updatePixDiffs()'.
    static std::map<std::pair<int, int>, nch::Color> getPixDiffs();

    /// @brief Specify an area of the screen to NOT track pixel changes during 'updatePixDiffs' calls. Makes that function less expensive.
    /// @param r A new rectangular area to ignore.
    static void addIgnoredPixSet(const nch::Rect& r);
    /// @brief Reset all rectangles (clear 'ignoredPixAreas') added by 'addIgnoredPixSet()'.
    static void resetPixSet();
private:
    static bool initted;
    static SDL_Texture* screenTex;
    static SDL_Surface* screenSurf;
    static SDL_Renderer* rend;

    static XImage* ximg;
    static Display* disp;
    static XShmSegmentInfo shmSegInfo;
    static nch::Rect dispArea;

    static std::set<std::pair<int, int>> pixSet;
    static std::vector<nch::Rect> ignoredPixAreas;
    static std::map<std::pair<int, int>, nch::Color> pixDiffs;
};