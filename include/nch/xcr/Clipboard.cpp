#include "Clipboard.h"
#include <nch/cpp-utils/log.h>

clipboard_c* Clipboard::cb = nullptr;

void Clipboard::open()
{
    if(cb!=nullptr) { nch::Log::warn(__PRETTY_FUNCTION__, "Clipboard is already open"); }

    cb = clipboard_new(NULL);
    if(cb==NULL) {
        nch::Log::error(__PRETTY_FUNCTION__, "Clipboard initialization failed");
    }
}

void Clipboard::close()
{
    if(cb==nullptr) { nch::Log::warn(__PRETTY_FUNCTION__, "Clipboard is already closed"); }

    clipboard_free(cb);
    cb = nullptr;
}

void Clipboard::set(std::string text) { clipboard_set_text(cb, text.c_str()); }
std::string Clipboard::get()  {
    std::stringstream ss;
    int len = 0;
    ss << clipboard_text_ex(cb, &len, LCB_CLIPBOARD);
    return ss.str();
}