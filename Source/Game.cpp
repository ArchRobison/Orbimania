/* Copyright 1996-2016 Arch D. Robison

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

/******************************************************************************
 OS-independent game code that is called from OS specific services on the host.
*******************************************************************************/

#include "AssertLib.h"
#include "Config.h"
#include "Clut.h"
#include "NimbleDraw.h"
#include "File.h"
#include "Game.h"
#include "Host.h"
#include "Menu.h"
#include "BuiltFromResource.h"
#include "Handle.h"
#include "PotentialField.h"
#include "Universe.h"
#include "Utility.h"
#include "View.h"
#include <cstdlib>
#include <cmath>

static int WindowWidth, WindowHeight, PanelWidth;
static float CurrentMousePointX, CurrentMousePointY;    // Current location of mouse in Universe coordinates

#define PROFILE_BUILD 0

#if PROFILE_BUILD
static int FrameCount = 0;
#endif
#if 0
static const int ClickableSetMaxSize = 4;
static Clickable* ClickableSet[ClickableSetMaxSize];
static Clickable** ClickableSetEnd = ClickableSet;

void DrawClickable( Clickable& clickable, NimblePixMap& map, int x, int y ) {
    Assert( ClickableSetEnd<&ClickableSet[ClickableSetMaxSize]);
    *ClickableSetEnd++ = &clickable;
    clickable.drawOn(map,x,y);
}
#endif

static struct MenuItemNewType: MenuItem {
    MenuItemNewType() : MenuItem("New") {}
    void onSelect() override {
		Universe::SetToDefaultParticleArrangment();
    }
} MenuItemNew;

static struct MenuItemOpenType : MenuItem {
    MenuItemOpenType() : MenuItem("Open...") {}
    void onSelect() override {
        std::string filename = HostGetFileName(HostGetFileNameOp::open, "Orbimania", "orbi");
		if( !filename.empty())
			ReadUniverseFromFile(filename);
    }
} MenuItemOpen;

static struct MenuItemSaveType : MenuItem {
    MenuItemSaveType() : MenuItem("Save As...") {}
    void onSelect() override {
		std::string filename = HostGetFileName(HostGetFileNameOp::saveAs, "Orbimania", "orbi");
		if( !filename.empty() )
			WriteUniverseToFile(filename);
	}
} MenuItemSave;

static Menu FileMenu("File", {&MenuItemNew, &MenuItemOpen, &MenuItemSave});

static bool IsRunning = true;
static void (*DrawPotentialField)(const NimblePixMap& map) = DrawPotentialFieldBilinear;

void GameUpdateDraw( NimblePixMap& map, NimbleRequest request ) {
    if(request & NimbleUpdate ) {
        if( IsRunning )
            AdvanceUniverseOneTimeStep();
    }
    if(request & NimbleDraw) {
        DrawPotentialField(map);
        extern void DrawFuturePaths(NimblePixMap& map);
        DrawFuturePaths(map);
        DrawMarkup(map);
        FileMenu.draw(map,0,0);
    }
#if PROFILE_BUILD
    if( ++FrameCount>=50 )
        HostExit();
#endif
}

static float Rand() {
    return float(rand())*(1.0f/RAND_MAX);
}

static void AddRandomParticle() {
    using namespace Universe;
    size_t k = NParticle;
    if(k<N_PARTICLE_MAX) {
        Charge[k] =  Rand()<0.5 ? 1 : -1;
        Mass[k] = 1;
        Sx[k] = Rand()+0.25;
        Sy[k] = Rand()+0.25;
        float theta = Rand()*(2*3.1415926535897932384626);
        float r = Rand();
        Vx[k] =  r*sin(theta);
        Vy[k] = r*cos(theta);
        ++NParticle;
    }
}

bool GameInitialize() {
    BuiltFromResourcePixMap::loadAll();

    using namespace Universe;
    InitializeClut();

#if 0
    // Test pattern consisting of NxN grid of particles
    IsRunning = false;
    const int N=31;
    NParticle = N*N;
    Assert(NParticle<=N_PARTICLE_MAX);
    for( int k=0; k<NParticle; ++k ) {
        int i = k%N;
        int j = k/N;
        Charge[k] = 0.05 * ((i^j)&1 ? 1 : -1);
        Sx[k] = (j+1)*(1./(2+N));
        Sy[k] = (i+1)*(1./(2+N));
        Vx[k] = 0;
        Vy[k] = 0;
        Mass[k] = 1;
    }
#elif !PROFILE_BUILD
	SetToDefaultParticleArrangment();
#else
    // For profiling
    NParticle = 0;
    for( size_t k=0; k<N_PARTICLE_MAX; ++k ) {
        AddRandomParticle();
    }
    SetZoom(ZoomLevel-1, -0.5*WindowWidth, -0.5*WindowHeight);
    ViewOffsetX -= 1;
    ViewOffsetY -= 0.25;
#endif
#if 0
    srand(2);
#else
    srand( unsigned( fmod( HostClockTime()*1E3, 4*double(1<<30))));
#endif
    InitMenu();
    return true;
}

void GameResizeOrMove( NimblePixMap& map ) {
    WindowWidth = map.width();
    WindowHeight = map.height();
}

const char* GameTitle() {
    return "Orbimania 2.0"
#if ASSERTIONS
           " ASSERTIONS"
#endif
    ;
}

static void ReverseDirection() {
    using namespace Universe;
    for( size_t k=0; k<NParticle; ++k ) {
        Vx[k] *= -1;
        Vy[k] *= -1;
    }
}

void FlipSelectedHandle() {
    using namespace Universe;
    size_t k = SelectedHandle.index;
    switch(SelectedHandle.kind) {
        case Handle::head: {
            Vx[k] *= -1;
            Vy[k] *= -1;
            break;
        }
        case Handle::tailHollow: {
            Charge[k] *= -1;
            break;
        }
        case Handle::circle: {
            Mass[k] *= -1;
            break;
        }
    }
}

void DeleteSelectedHandle() {
    switch( SelectedHandle.kind ) {
        case Handle::tailFull: {
            size_t k = SelectedHandle.index;
            SelectedHandle = Handle();
            Universe::EraseParticle(k);
            UpdateHandlesAfterErasure(k);
            break;
        }
        default:
            break;
    }
}

static struct Clipboard {
    // True if not empty
    bool full;  
    Universe::Float vx, vy, mass, charge;
} TheClipBoard;

static void CopySelectedHandle() {
    using namespace Universe;
    switch(SelectedHandle.kind) {
        case Handle::tailFull: {
            size_t k = SelectedHandle.index;
            auto& c = TheClipBoard;
            c.vx = Vx[k];
            c.vy = Vy[k];
            c.mass = Mass[k];
            c.charge = Charge[k];
            c.full = true;
            break;
        }
        default:
            TheClipBoard.full = false;
            break;
    }
}

static void PasteSelectedHandle() {
    using namespace Universe;
    if( TheClipBoard.full && NParticle<N_PARTICLE_MAX) {
        size_t k = NParticle++;
        auto& c = TheClipBoard;
        Sx[k] = CurrentMousePointX;
        Sy[k] = CurrentMousePointY;
        Vx[k] = c.vx;
        Vy[k] = c.vy;
        Charge[k] = c.charge;
        Mass[k] = c.mass;
    }
}

void GameKeyDown( int key ) {
    Assert( !('A'<=key && key<='Z') );  // Alphabetic key should be lower case.
    switch(key) {
        case HOST_KEY_ESCAPE:
            HostExit();
            return;
        case ' ':
            IsRunning = !IsRunning;
            break;
        case '-':
            SetZoom(ZoomLevel-1, WindowWidth/2, WindowHeight/2);
            break;
        case '=':
            SetZoom(ZoomLevel+1, WindowWidth/2, WindowHeight/2);
            break;
        case '0':
            SetZoom(0, WindowWidth/2, WindowHeight/2);
            break;
        case '9':
            Universe::Recenter();
            RecenterView(WindowWidth/2, WindowHeight/2);
            break;
        case 'm': 
            AddRandomParticle();
            break;
        case 'b': 
            DrawPotentialField = DrawPotentialFieldBilinear;
            break;
        case 'g': 
            DrawPotentialField = DrawPotentialFieldPrecise;
            break;
        case 'h': 
            DrawPotentialField = DrawPotentialFieldBarnesHut;
            break;
        case 'r':
            ReverseDirection();
            break;
        case 'f':
            FlipSelectedHandle();
            break;
        case HOST_KEY_DELETE:
            DeleteSelectedHandle();
            break;
        case 'c':
            CopySelectedHandle();
            break;
        case 'v':
            PasteSelectedHandle();
            break;
        case 'x':
            CopySelectedHandle();
            DeleteSelectedHandle();
            break;
#if 0
        case HOST_KEY_RETURN:
            VisibleDialog = NULL;
            break;
        case 'f':
            ShowFrameRate = !ShowFrameRate;
            break;
        case 'n':
            if( ScoreState.isTraining() )
                CreateNewArea();
            else
                ScoreState.startNewArea();
            break;
        case 'p':
            IsPaused.toggleChecked();
            break;
#endif
        default:
            break;
    }
}

static float DownMousePointX, DownMousePointY, DownMouseScalar;

void GameMouse( MouseEvent e, const NimblePoint& point ) {
    using namespace Universe;
    float x = point.x;
    float y = point.y;
    if( FileMenu.trackMouse(e, x, y ) ) {
		SelectedHandle = Handle(); 
		DownMouseScalar = 0;
        return;
    }
    CurrentMousePointX = ViewScale*x + ViewOffsetX;
    CurrentMousePointY = ViewScale*y + ViewOffsetY;
    switch( e ) {
        case MouseEvent::down:
            DownMousePointX = CurrentMousePointX;
            DownMousePointY = CurrentMousePointY;
            DownMouseScalar = 0;
            SelectHandle(x, y);
            switch(SelectedHandle.kind) {
                case Handle::circle: {
                    DownMouseScalar = Mass[SelectedHandle.index];
                    break;
                }
                case Handle::tailHollow:
                    DownMouseScalar = Charge[SelectedHandle.index];
                    break;
                default:
                    break;
            }
            break;
        case MouseEvent::up:
            break;
        case MouseEvent::move:
            SelectHandle(x, y);
            break;
        case MouseEvent::drag:
            switch(SelectedHandle.kind) {
                case Handle::null: {
                    ViewOffsetX = DownMousePointX - ViewScale*point.x;
                    ViewOffsetY = DownMousePointY - ViewScale*point.y;
                    break;
                }
                case Handle::tailFull: {
                    size_t k = SelectedHandle.index;
                    Sx[k] = ViewScale*x + ViewOffsetX;
                    Sy[k] = ViewScale*y + ViewOffsetY;
                    break;
                }
                case Handle::head: {
                    size_t k = SelectedHandle.index;
                    float tailX = (Sx[k] - ViewOffsetX)/ViewScale;
                    float tailY = (Sy[k] - ViewOffsetY)/ViewScale;
                    Vx[k] = ViewVelocityScale * (x-tailX);
                    Vy[k] = ViewVelocityScale * (y-tailY);
                    break;
                }
                case Handle::circle:
                case Handle::tailHollow: {
                    size_t k = SelectedHandle.index;
                    // Center of object in universe
                    float cx = Sx[k];
                    float cy = Sy[k];
                    // Original mouse positon in universe
                    float qx = DownMousePointX;
                    float qy = DownMousePointY;
                    // Current mouse position in universe
                    float px = ViewScale*x + ViewOffsetX;
                    float py = ViewScale*y + ViewOffsetY;
                    float dot = (px-cx)*(DownMousePointX-cx) + (py-cy)*(DownMousePointY-cy);
                    float ratio = std::sqrt(Dist2(px, py, cx, cy)/Dist2(qx, qy, cx, cy)) * (dot>=0 ? 1 : -1);
                    if(SelectedHandle.kind==Handle::circle) {
                        Mass[k] = ratio*DownMouseScalar;
                    } else {
                        Charge[k] = ratio*DownMouseScalar;
                    }
                    break;
                }
            }
            break;
    }
}

