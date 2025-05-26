#pragma once
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
/**/
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include "nch/cpp-utils/color.h"
#include "nch/math-utils/vec2.h"
#include "nch/sdl-utils/rect.h"

namespace nch { class XTools {
public:
    /* Getters */
    /// @brief Get the absolute (x, y) position of the mouse cursor.
    /// @return The current position of the mouse, as a 2D vector (x, y).
    static nch::Vec2i getMouseXY();
    /// @brief Get an nch::Rect object matching up with the position and dimensions of the window with ID 'windowID'.
    /// @param xWindowID The ID of the window.
    /// @return A rectangle representing the position+dimensions of the window.
    static nch::Rect getWindowRect(int WindowID);
    /// @brief Finds all visible X11 windows whose window title/classname strings match with the provided 'substr'.
    /// @param substr The substring to match against the X11 window titles/classes.
    /// @return A list of ints corresponding to the IDs of the windows found.
    static std::vector<int> findWindowIDsByTitle(std::string substr); static std::vector<int> findWindowIDsByClassName(std::string substr);
    /// @brief Same as findWindowIDs but only return the first result or -1 if there were no results.
    /// @param substr The substring to match against the X11 window titles/classes.
    /// @return An int corresponding to the window ID found.
    static int getWindowIDByTitle(std::string substring); static int getWindowIDByClassName(std::string substr);
    /// @brief Try to return the ID of the current active window. Upon failure, return -1.
    /// @return ID of the current active window.
    static int getActiveWindowID();
    /// @brief Given a character, return the xdotool key code associated with typing that character. Returns "" and a warning if an unknown char is inputted.
    /// @brief Supported characters: any char in " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ`~1!2@3#4$5%6^7&8*9(0)-_=+/[]\\|;:'\",<.>?\n\t"
    /// @param c The character to get the key code from.
    /// @return The xdotool key code (a string) associated with 'c'.
    static std::string charToKeyCode(char c);
    
    /* Mutators */
    /// @brief Simulate "natural typing" (as if by a human) of a string of characters using xdotool.
    /// @brief There are small amounts of random delay in between keystrokes (may help with Cloudflare/bot detection).
    /// @brief See charToXKeyCode() for a list of supported characters.
    /// @param toType The string to type using xdotool keyboard strokes, as if in a textbox.
    static void naturallyTypeString(std::string toType);
    /// @brief Simulate a "natural keystroke" (as if by a human).
    /// @brief There is a random, small delay after keydown and keyup.
    /// @param keycode The xdotool keycode (or multiple, such as "ctrl+v") to use for the keystroke.
    static void naturalKeystroke(std::string keycode);
    static void naturalKeystrokes(std::vector<std::string> keycodeSet);
    /// @brief Set the absolute (x, y) position of the mouse cursor.
    /// @param pos A 2D vector representing the new position the mouse cursor should be at.
    static void setMouseXY(const nch::Vec2i& xy);
    /// @brief Do a mouse press and a mouse release in quick succession.
    /// @param btn The mouse button to press and release
    static void mouseClick(int btn = 1);
    static void activateWindow(int winID);
    static void maximizeWindow(int winID, nch::Vec2i maximizeButtonPos);
    static void setWindowTitle(int winID, std::string newWinTitle);
private:
}; }