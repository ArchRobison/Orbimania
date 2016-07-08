#include <Windows.h>
#include "AssertLib.h"
#include "Host.h"

class StringBuilder {
    char* ptr;
public:
    StringBuilder(char * dst) : ptr(dst) {}
    StringBuilder& append(const char* src, bool skipnull=false);
};

StringBuilder& StringBuilder::append(const char* src, bool skipnull) {
    while(*ptr = *src++)
        ++ptr;
    if(skipnull)
        ++ptr;
    return *this;
}

std::string HostGetFileName(HostGetFileNameOp op, const char* fileType, const char* fileSuffix) {
    std::string result;
    char buffer[MAX_PATH+1] = "";
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    extern HWND HostMainWindowHandle;  // Defined in Host_sdl.cpp
    ofn.hwndOwner = HostMainWindowHandle;
    ofn.nMaxFile = sizeof(buffer);
    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
    ofn.lpstrFile = buffer;
    char filter[128+5];                                 //+5 is for 3 nulls and "*."
    Assert(strlen(fileType)+strlen(fileSuffix)+3 <= sizeof(filter));
    StringBuilder(filter).append(fileType, true).append("*.").append(fileSuffix, true).append("");
    ofn.lpstrFilter = filter;
    switch(op) {
        default:
            Assert(false);  // Illegal
            break;
        case HostGetFileNameOp::open:
            ofn.lpstrTitle = "Open";
            ofn.Flags |= OFN_FILEMUSTEXIST;
            if(GetOpenFileName(&ofn)) {
                result = buffer;
            }
            break;
        case HostGetFileNameOp::create:
        case HostGetFileNameOp::saveAs:
            ofn.lpstrTitle = op==HostGetFileNameOp::create ? "Create" : nullptr;
            if(GetSaveFileName(&ofn)) {
                result = buffer;
            } else {
#if 0
                // Sometimes helpfule for debugging.
                DWORD x = CommDlgExtendedError();
                switch(x) {
                    case CDERR_DIALOGFAILURE:
                        break;
                    case CDERR_INITIALIZATION:
                        break;
                }
#endif
            }
            break;
    }
    return result;
}
