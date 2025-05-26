#pragma once
#include <libclipboard.h>
#include <string>

class Clipboard {
public:
    static void open();
    static void close();

    static void set(std::string text);
    static std::string get();
private:

    static clipboard_c* cb;
};