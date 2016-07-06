/* Copyright 2014-2016 Arch D. Robison

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// Interfaces that isolate application from host dependencies.

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "../../Source/Host.h"
#include "../../Source/Game.h"
#include "../../Source/BuiltFromResource.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#endif

static SDL_PixelFormat* ScreenFormat;

#ifdef __APPLE__
// MacOS/SDL-2 have a texture synchronization bug on MacBook Airs running MacOS 10.11.1.
// The code works around the bug by manual double-buffering.
const int N_TEXTURE = 2;
#else
const int N_TEXTURE = 1;
#endif

#if _MSC_VER >= 1900
// Work-around for SDL2 compiled against VS2013
FILE _iob[] ={ *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func(void) {
    return _iob;
}
#endif

static void ReportResourceError( const char* routine, const char* path, const char* error ) {
    char* cwd = getcwd(nullptr, 0);
    std::printf("Current working directory = %s\n", cwd);
    std::free(cwd);
    std::fprintf( stderr, "Internal error: %s failed %s: %s\n", routine, path, error );
    Assert(false);
    HostExit();
}

static std::string ResourceFilePath(const char* resourceName, const char* extension) {
#if defined(HOST_RESOURCE_PATH)
    std::string path(HOST_RESOURCE_PATH);
#else
    std::string path("../../../Resource");
#endif
    return path + "/" + resourceName + "." + extension;
}

void HostLoadResource(BuiltFromResourcePixMap& item) {
    std::string path = ResourceFilePath(item.resourceName(), "png");
    if( SDL_Surface* raw = IMG_Load(path.c_str()) ) {
        if( SDL_Surface* image = SDL_ConvertSurface(raw, ScreenFormat, 0) ) {
            SDL_FreeSurface(raw);
            if(SDL_LockSurface(image)==0) {
                NimblePixMap map(image->w, image->h, 8*sizeof(NimblePixel), image->pixels, image->pitch);
                item.buildFrom(map);
                SDL_UnlockSurface(image);
            } else {
                ReportResourceError("SDL_LockSurface", item.resourceName(), SDL_GetError());
            }
            SDL_FreeSurface(image);
        } else {
            ReportResourceError("SDL_ConvertSurface", item.resourceName(), SDL_GetError());
        }
    } else {
        ReportResourceError("IMG_Load", path.c_str(), IMG_GetError());
    }
}

double HostClockTime() {
    return SDL_GetTicks()*0.001;
}

static float BusyFrac;

float HostBusyFrac() {
    return BusyFrac;
}

static int NewFrameIntervalRate=1, OldFrameIntervalRate=-1;

void HostSetFrameIntervalRate(int limit) {
    NewFrameIntervalRate = limit;
}

static bool Quit;
static bool Resize = true;

void HostExit() {
    Quit = true;
}

// [i] has SDL_Scancode corresponding to HOST_KEY_...
static unsigned short ScanCodeFromHostKey[HOST_KEY_LAST];

// [i] has HOST_KEY_... corresponding to SDL_ScanCode
static unsigned short HostKeyFromScanCode[SDL_NUM_SCANCODES];

inline void Associate(SDL_Scancode code,int key) {
    Assert(unsigned(code)<SDL_NUM_SCANCODES);
    Assert(unsigned(key)<HOST_KEY_LAST);
    ScanCodeFromHostKey[key] = code;
    HostKeyFromScanCode[code] =key;
}

bool HostIsKeyDown(int key) {
    static int numKeys;
    static const Uint8* state = SDL_GetKeyboardState(&numKeys);
    int i = ScanCodeFromHostKey[key];
    return i<numKeys ? state[i] : false;
}

static void InitializeKeyTranslationTables() {
#if 0
    for( int i=' '; i<='@'; ++i )
        KeyTranslate[i] = i;
#endif
    Associate(SDL_SCANCODE_SPACE, ' ');
    for( int i='a'; i<='z'; ++i )
        Associate(SDL_Scancode(SDL_SCANCODE_A+(i-'a')), i);
    Associate(SDL_SCANCODE_9, '9');
    Associate(SDL_SCANCODE_0, '0');
    Associate(SDL_SCANCODE_EQUALS, '=');
    Associate(SDL_SCANCODE_MINUS, '-');
    Associate(SDL_SCANCODE_RETURN, HOST_KEY_RETURN);
    Associate(SDL_SCANCODE_ESCAPE, HOST_KEY_ESCAPE);
    Associate(SDL_SCANCODE_LEFT, HOST_KEY_LEFT);
    Associate(SDL_SCANCODE_RIGHT, HOST_KEY_RIGHT);
    Associate(SDL_SCANCODE_UP, HOST_KEY_UP);
    Associate(SDL_SCANCODE_DOWN, HOST_KEY_DOWN);
    Associate(SDL_SCANCODE_DELETE, HOST_KEY_DELETE);
}

static void PollEvents() {
    SDL_Event event;
    static MouseEvent mouseMoveEvent;
    while(SDL_PollEvent(&event)) {
        switch(event.type){
            /* Keyboard event */
            /* Pass the event data onto PrintKeyInfo() */
            case SDL_KEYDOWN:
                /*case SDL_KEYUP:*/
                GameKeyDown(HostKeyFromScanCode[event.key.keysym.scancode]);
                break;

                /* SDL_QUIT event (window close) */
            case SDL_MOUSEMOTION:
                GameMouse(mouseMoveEvent, NimblePoint(event.motion.x, event.motion.y));
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouseMoveEvent = MouseEvent::drag;
                GameMouse(MouseEvent::down, NimblePoint(event.button.x, event.button.y));  
                break;
            case SDL_MOUSEBUTTONUP:
                mouseMoveEvent = MouseEvent::move;
                GameMouse(MouseEvent::up, NimblePoint(event.button.x, event.button.y));
                break;
            case SDL_QUIT:
                Quit = true;
                break;

            default:
                break;
        }
    }
}

static const bool UseRendererForUnlimitedRate = true;

//! Destroy renderer and texture, then recreate them if they are to be used.  Return true if success; false if error occurs.
static bool RebuildRendererAndTexture(SDL_Window* window , int w, int h, SDL_Renderer*& renderer, SDL_Texture* texture[N_TEXTURE]) {
    for( int i=0; i<N_TEXTURE; ++i )
        if( texture[i] ) {
            SDL_DestroyTexture(texture[i]);
            texture[i] = nullptr;
        }
    if( renderer ) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if( UseRendererForUnlimitedRate || NewFrameIntervalRate>0 ) {
        Uint32 flags = SDL_RENDERER_ACCELERATED;
        if( NewFrameIntervalRate>0 )
            flags |= SDL_RENDERER_PRESENTVSYNC;
        renderer = SDL_CreateRenderer(window, -1, flags);
        if( !renderer ) {
            printf("Internal error: SDL_CreateRenderer failed: %s\n", SDL_GetError());
            return false;
        }
        for( int i=0; i<N_TEXTURE; ++i ) {
            texture[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
            if( !texture[i] ) {
                printf("Internal error: SDL_CreateRenderer failed: %s\n", SDL_GetError());
                return false;
            }
        }
    }
    OldFrameIntervalRate = NewFrameIntervalRate;
    return true;
}

int main(int argc, char* argv[]){
    if(SDL_Init(SDL_INIT_VIDEO) == -1){
        printf("Internal error: SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
    SDL_DisplayMode displayMode;
    if(SDL_GetCurrentDisplayMode(0, &displayMode)) {
        printf("Internal error: SDL_GetCurrentDisplayMode failed: %s\n", SDL_GetError());
        exit(1);
    }
    int w = displayMode.w;
    int h = displayMode.h;
#if !EXCLUSIVE_MODE
    w = 768;
    h = 768;
#endif
    SDL_Window* window = SDL_CreateWindow(
        GameTitle(),
        SDL_WINDOWPOS_UNDEFINED,    // initial x position
        SDL_WINDOWPOS_UNDEFINED,    // initial y position
        w,
        h,
#if EXCLUSIVE_MODE
        SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN
#else
        SDL_WINDOW_SHOWN
#endif
        );
    InitializeKeyTranslationTables();
    ScreenFormat = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    if(GameInitialize()) {
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* texture[N_TEXTURE];
        for( int i=0; i<N_TEXTURE; ++i )
            texture[i] = nullptr;
        int textureIndex = 0;
        while(!Quit) {
            if(NewFrameIntervalRate!=OldFrameIntervalRate)
                if(!RebuildRendererAndTexture(window, w, h, renderer, texture))
                    break;
            void* pixels;
            int pitch;
            if( !texture[textureIndex] ) {
                fprintf(stderr,"No texture!\n");
                abort();
            }
            if(SDL_LockTexture(texture[textureIndex], nullptr, &pixels, &pitch)) {
                printf("Internal eror: SDL_LockTexture failed: %s\n", SDL_GetError());
                break;
            }
            double t0 = HostClockTime();
            NimblePixMap screen(w, h, 8*sizeof(NimblePixel), pixels, pitch);
            if(Resize) {
                GameResizeOrMove(screen);
                Resize = false;
            }
            GameUpdateDraw(screen, NimbleUpdate|NimbleDraw);
#if 0
            extern void ThrottleWorkers(double,double);
            ThrottleWorkers(t0,HostClockTime());
#endif
            SDL_UnlockTexture(texture[textureIndex]);
            SDL_RenderClear(renderer);
            // Assume 60 Hz update rate.  Simulate slower refresh rate by presenting texture twice.
            // At least one trip trhough the loop is required because a rate of 0 indicates "unlimited".
            int i = 0;
            do {
                SDL_RenderCopy(renderer, texture[textureIndex], nullptr, nullptr);
                SDL_RenderPresent(renderer);
            } while( ++i<OldFrameIntervalRate );
            PollEvents();
            textureIndex = (textureIndex + 1) & N_TEXTURE-1;
        }
        for( int i=0; i<N_TEXTURE; ++i )
            if(texture[i])
                SDL_DestroyTexture(texture[i]);
        if(renderer) SDL_DestroyRenderer(renderer);
    } else {
        printf("GameInitialize() failed\n");
        return 1;
    }

    SDL_DestroyWindow(window);
    return 0;
}

const char* resourceDir = "../";

void HostFont::open(const char* fontname, int ptsize) {
    if( !TTF_WasInit() ) {
        TTF_Init();
    }
    std::string path = ResourceFilePath(fontname, "ttf");
    TTF_Font* font = TTF_OpenFont(path.c_str(), ptsize);
    if( !font ) {
        ReportResourceError("TTF_OpenFont", path.c_str(), TTF_GetError());
    }
    myFont = font;
}

// Close frees resources used by the given Font object.
void HostFont::close() {
    if( myFont!=nullptr ) {
        TTF_CloseFont(static_cast<TTF_Font*>(myFont));
        myFont = nullptr;
    }
}

// DrawText draws the given text at (x,y) on pm, in the given color and font.
// The return value indicates the width and height of a bounding box for the text.
NimbleRect HostFont::draw(NimblePixMap& map, int x, int y, const char* text, const NimbleColor& color) const {
    TTF_Font* font = static_cast<TTF_Font*>(myFont);
    if(text[0]) {
        SDL_Color c;
        c.r = color.red;
        c.g = color.green;
        c.b = color.blue;
        c.a = color.alpha;
        if(SDL_Surface* tmp = TTF_RenderUTF8_Solid(font, text, c)) {
            int width = tmp->w;
            int height = tmp->h;
            if(SDL_Surface* dst = SDL_CreateRGBSurfaceFrom(map.at(0, 0), map.width(), map.height(), 32, map.bytesPerRow(),
                0xFF<<NimbleColor::redShift,
                0xFF<<NimbleColor::greenShift,
                0xFF<<NimbleColor::blueShift,
                0xFF<<NimbleColor::alphaShift)) {
                SDL_Rect dstRect;
                dstRect.x = x;
                dstRect.y = y;
                dstRect.w = width;
                dstRect.h = height;
                if(SDL_BlitSurface(tmp, nullptr, dst, &dstRect)) {
                    std::fprintf(stderr, "SDL_CreateRGBSurfaceFrom failed: %s\n", TTF_GetError());
                    Assert(false);
                }
                SDL_FreeSurface(dst);
            } else {
                std::fprintf(stderr, "SDL_CreateRGBSurfaceFrom failed: %s\n", TTF_GetError());
                Assert(false);
            }
            SDL_FreeSurface(tmp);
            return NimbleRect(x, y, x+width, y+height);
        } else {
            std::fprintf(stderr, "TTF_RenderUTF8_Solid failed: %s\n", TTF_GetError());
            Assert(false);
        }
    }
    return NimbleRect(x, y, x, y+height());
}

// Height returns the nominal height of the font.
int HostFont::height() const {
    TTF_Font* font = static_cast<TTF_Font*>(myFont);
    return TTF_FontHeight(font);
}

NimblePoint HostFont::size(const char* text) const {
    TTF_Font* font = static_cast<TTF_Font*>(myFont);
    int w=0, h=0;
    if( TTF_SizeUTF8(font, text, &w, &h) ) {
        fprintf(stderr,"HostFont::size failed for %s\n",text);
        Assert(false);
    }
    return NimblePoint(w,h);
}
