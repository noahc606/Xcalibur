#include "MiscTools.h"
#include <SDL2/SDL_image.h>
#include <QApplication>
#include <QClipboard>
#include <QGuiApplication>
#include <assert.h>
#include <libclipboard/libclipboard.h>
#include <nch/cpp-utils/file-utils.h>
#include <nch/cpp-utils/shell.h>
#include <nch/cpp-utils/string-utils.h>
#include <nch/cpp-utils/log.h>
#include "Xcalibur.h"

using namespace nch;

clipboard_c* cb = nullptr;

void MiscTools::globalInitLibclipboard()
{
    if(cb!=nullptr) { nch::Log::warn(__PRETTY_FUNCTION__, "Clipboard is already open"); }
    cb = clipboard_new(NULL);
    if(cb==NULL) {
        nch::Log::error(__PRETTY_FUNCTION__, "Clipboard initialization failed");
    }
}
void MiscTools::globalFreeLibclipboard()
{
    if(cb==nullptr) { nch::Log::warn(__PRETTY_FUNCTION__, "Clipboard is already closed"); }
    clipboard_free(cb);
    cb = nullptr;
}

std::string MiscTools::qtGetClipboard()
{
    int argc = 1;
    char* args[] = { (char*)"TempApp" };
    QApplication app(argc, args);
    
    QClipboard* qcb = QGuiApplication::clipboard();
    if(qcb!=nullptr) {
        QString text = qcb->text();
        return text.toStdString();
    }

    Log::errorv(__PRETTY_FUNCTION__, "QGuiApplication::clipboard()", "clipboard is nullptr");
    return "???null???";
}
void MiscTools::lcSetClipboard(const std::string& clipboardText)
{
    if(cb==nullptr) {
        nch::Log::warn(__PRETTY_FUNCTION__, "Clipboard is not open");
        return;
    }

    if(clipboardText=="") {
        clipboard_set_text(cb, "");
        clipboard_clear(cb, clipboard_mode::LCB_CLIPBOARD);
        clipboard_clear(cb, clipboard_mode::LCB_MODE_END);
        clipboard_clear(cb, clipboard_mode::LCB_PRIMARY);
        clipboard_clear(cb, clipboard_mode::LCB_SECONDARY);
        clipboard_clear(cb, clipboard_mode::LCB_SELECTION);
    } else {
        clipboard_set_text(cb, clipboardText.c_str());
    }
    
}

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

std::vector<Rect> MiscTools::displayFindTextboxes(const Rect& displayArea, std::string textToFind)
{
    /* Input checking */
    if(textToFind.size()==0) return {};
    //Get rid of all 
    std::stringstream ss;
    for(int i = 0; i<textToFind.size(); i++) {
        if(textToFind[i]!=' ') {
            ss << textToFind[i];
        }
    }
    textToFind = ss.str();
    std::vector<Rect> res;

    /* Get the contents of the .box file generated from Tesseract's OCR */
    std::vector<std::string> fileLines;
    {
        //Generate image from 'displayArea'
        auto surf = Xcalibur::displayToSDLSurf(displayArea);
        IMG_SavePNG(surf, "temp_textbox_screencap.png");
        SDL_FreeSurface(surf);
        //Perform OCR on image and get the conents of the .box file
        Shell::exec("tesseract temp_textbox_screencap.png temp_textbox_screencap makebox");
        FILE* fp = fopen("temp_textbox_screencap.box", "r");
        fileLines = FileUtils::getFileLines(fp);    
        fclose(fp);
    }

    //Get a list of all characters detected by the OCR (left -> right, top -> bottom order)
    std::stringstream ocrStream;
    for(int i = 0; i<fileLines.size(); i++) {
        if(fileLines[i].size()>0)
            ocrStream << fileLines[i].at(0);
    }

    //Build 'textbox', if it exists...
    //Repeatedly find location of the 'textToFind' within the 'ocrStream'
    for(size_t txtI = ocrStream.str().find(textToFind); txtI!=std::string::npos; txtI = ocrStream.str().find(textToFind, txtI+1)) {
        Rect textbox = Rect(-1, -1, 0, 0);
        if(txtI!=std::string::npos) {
            auto firstCoords = StringUtils::parseI64ArraySimple(fileLines[txtI]);
            textbox = Rect::createFromTwoPts(firstCoords[1], firstCoords[2], firstCoords[3], firstCoords[4]);
    
            for(size_t i = txtI; i<txtI+textToFind.size(); i++) {
                auto iCoords = StringUtils::parseI64ArraySimple(fileLines[i]);
                int x1 = iCoords[1];
                int y1 = iCoords[2];
                int x2 = iCoords[3];
                int y2 = iCoords[4];
                
                //Expand textbox until it fits all the desired characers within 'textToFind'.
                if(x1<textbox.x1()) { textbox.r.x = x1; }
                if(y1<textbox.y1()) { textbox.r.y = y1; }
                if(x2>textbox.x2()) { textbox.r.w += (x2-textbox.x2()); }
                if(y2>textbox.y2()) { textbox.r.h += (y2-textbox.y2()); }
            }
        }

        Rect textboxModded = textbox;
        textboxModded.r.y = displayArea.r.h-textbox.r.y-textbox.r.h;
        textbox = textboxModded;

        res.push_back(textbox);
    }

    //Return
    return res;
}