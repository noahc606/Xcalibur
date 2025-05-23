#include "XTools.h"
#include <nch/cpp-utils/log.h>
#include <nch/cpp-utils/string-utils.h>
#include <nch/cpp-utils/timer.h>
#include <nch/sdl-utils/texture-utils.h>
#include <SDL2/SDL_image.h>
#include <sys/shm.h>
#include "MiscTools.h"
#include "Shell.h"

using namespace nch;

Vec2i XTools::getMouseXY()
{
    Vec2i res;
    try {
        res.x = std::stoi( StringUtils::split(Shell::exec("xdotool getmouselocation"), ' ').at(0).substr(2) );
        res.y = std::stoi( StringUtils::split(Shell::exec("xdotool getmouselocation"), ' ').at(1).substr(2) );
    } catch(...) { res = {-1, -1}; }
    return res;
}

Rect XTools::getWindowRect(int windowID)
{
    StringUtils su;
    Rect res;
    res.r.x = -1; res.r.y = -1;
    res.r.w = -1; res.r.h = -1;
    
    std::stringstream cmd;
    cmd << "xwininfo -id " << windowID;
    std::vector<std::string> split = su.split(Shell::exec(cmd.str()), '\n');
    for(int i = 0; i<split.size(); i++) { try {
        std::string si = su.trimmed(split.at(i));
        if(su.aHasPrefixB(si, "Absolute upper-left X:")) {
            res.r.x = std::stoi(su.trimmed(si.substr(23)));
        }
        if(su.aHasPrefixB(si, "Absolute upper-left Y:")) {
            res.r.y = std::stoi(su.trimmed(si.substr(22)));
        }
        if(su.aHasPrefixB(si, "Width:")) {
            res.r.w = std::stoi(su.trimmed(si.substr(6)));
        }
        if(su.aHasPrefixB(si, "Height:")) {
            res.r.h = std::stoi(su.trimmed(si.substr(7)));
        }
    } catch(...) {} }
    
    return res;
}

std::vector<int> XTools::findWindowIDsByTitle(std::string regex)
{
    std::vector<int> res;
    if(!StringUtils::validateSafeString(regex)) return res;

    auto execRes = StringUtils::split(Shell::exec("xdotool search --onlyvisible --name '"+regex+"'"), '\n');
    for(std::string line : execRes) { try { res.push_back(std::stoi(line)); } catch(...) {} }
    return res;
}

std::vector<int> XTools::findWindowIDsByClass(std::string regex)
{
    std::vector<int> res;
    if(!StringUtils::validateInjectionless(regex)) return res;

    auto execRes = StringUtils::split(Shell::exec("xdotool search --onlyvisible --class '"+regex+"'"), '\n');
    for(std::string line : execRes) { try { res.push_back(std::stoi(line)); } catch(...) {} }
    return res;
}

int XTools::getWindowIDByTitle(std::string regex)
{
    try { return findWindowIDsByTitle(regex).at(0); } catch(...){}
    Log::warnv(__PRETTY_FUNCTION__, "returning -1", "No windows found with titles matching the regex \"%s\"", regex.c_str());
    return -1;
}
int XTools::getWindowIDByClass(std::string regex)
{
    try { return findWindowIDsByClass(regex).at(0); } catch(...){}
    Log::warnv(__PRETTY_FUNCTION__, "returning -1", "No windows found with classes matching the regex \"%s\"", regex.c_str());
    return -1;
}

int XTools::getActiveWindowID()
{
    try { return std::stoi(Shell::exec("xdotool getactivewindow")); } catch(...) {}
    Log::warnv(__PRETTY_FUNCTION__, "returning -1", "Failed to run \"xdotool getactivewindow\"");
    return -1;
}

std::string XTools::charToKeyCode(char c)
{
    //[0-9], [Aa-Zz]
    if(('0'<=c&&c<='9') || ('A'<=c&&c<='Z') || ('a'<=c&&c<='z')) {
        std::stringstream ss; ss << c; return ss.str();
    }
    //Non-alphanumeric
    switch(c) {
        case ' ': return "space";
        case '`': return "grave";
        case '~': return "asciitilde";
        case '!': return "exclam";
        case '@': return "at";
        case '#': return "numbersign";
        case '$': return "dollar";
        case '%': return "percent";
        case '^': return "asciicircum";
        case '&': return "ampersand";
        case '*': return "exclam";
        case '(': return "parenleft";
        case ')': return "parenright";
        case '-': return "minus";
        case '_': return "underscore";
        case '=': return "equal";
        case '+': return "plus";
        case '/': return "slash";
        case '[': return "bracketleft";
        case '{': return "braceleft";
        case ']': return "bracketright";
        case '}': return "braceright";
        case '\\': return "backslash";
        case '|': return "bar";
        case ';': return "semicolon";
        case ':': return "colon";
        case '\'': return "apostrophe";
        case '\"': return "quotedbl";
        case ',': return "comma";
        case '<': return "less";
        case '.': return "period";
        case '>': return "greater";
        case '?': return "question";
    }
    //Specials
    switch(c) {
        case '\n': return "Return";
        case '\t': return "Tab";
    }
    //Unknown
    Log::warnv(__PRETTY_FUNCTION__, "returning \"\"", "Key code for '%c' is unknown", c);
    return "";
}

void XTools::naturallyTypeString(std::string toType)
{
    /* Preparations */
    //Get the seed for the RNG within this function
    uint64_t sum = 0;
    for(int i = 0; i<toType.size(); i++) sum += toType[i];

    /* Execute the typing operation */
    for(int i = 0; i<toType.size(); i++) {
        //Get char and its keycode
        char c = toType[i];
        std::string kc = charToKeyCode(c);
        if(kc=="") continue;
        //RNG
        std::srand(sum+c);
        naturalKeystroke(kc);
    }
}

void XTools::naturalKeystroke(std::string keycode)
{
    int keyDownMS = 15+(std::rand()%15);
    int keyUpMS = 35+(std::rand()%55);
    //Randomly hold down key and release
    Shell::exec(std::string("xdotool keydown "+keycode));   Timer::sleep(keyDownMS);   //Key down for some amount of time
    Shell::exec(std::string("xdotool keyup "+keycode));     Timer::sleep(keyUpMS);     //Key up for some amount of time, then continue to next
}

void XTools::naturalKeystrokes(std::vector<std::string> keycodeSet)
{
    for(int i = 0; i<keycodeSet.size(); i++) {
        naturalKeystroke(keycodeSet[i]);
    }
}

void XTools::setMouseXY(const Vec2i& pos)
{
    if(pos.x<0 || pos.y<0) {
        Log::warnv(__PRETTY_FUNCTION__, "doing nothing", "Mouse position out of bounds");
        return;
    }

    std::stringstream cmd;
    cmd << "xdotool mousemove " << pos.x << " " << pos.y;
    Shell::exec(cmd.str());
}

void XTools::mouseClick(int btn)
{
    std::stringstream cmd;
    cmd << "xdotool mousedown " << btn << " && xdotool mouseup " << btn;
    Shell::exec(cmd.str());
}

void XTools::ensureWindowActivated(int winID)
{
    std::stringstream cmds;
    cmds << "xdotool windowactivate " << winID;
    Shell::exec(cmds.str());
}