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
#include "Game.h"
#include "Host.h"
#include "BuiltFromResource.h"
#include "Handle.h"
#include "Universe.h"
#include "Utility.h"
#include "View.h"
#include <cstdlib>
#include <cmath>

static int WindowWidth, WindowHeight, PanelWidth;

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

static bool IsRunning = true;

void GameUpdateDraw( NimblePixMap& map, NimbleRequest request ) { 
    if(request & NimbleUpdate ) {
        if( IsRunning )
            AdvanceUniverseOneTimeStep();
    }
    if(request & NimbleDraw) {
        DrawPotentialField(map);
        DrawMarkup(map);
    }
}

bool GameInitialize() {
    BuiltFromResourcePixMap::loadAll();

    using namespace Universe;
    InitializeClut();
    NParticle = 2;

    Charge[0] =  1; Mass[0] = 1; Sx[0] = 0.25; Sy[0] = 0.25; Vx[0] =  0.8; Vy[0] = -0.1;
    Charge[1] = -1; Mass[1] = 1; Sx[1] = 0.75; Sy[1] = 0.75; Vx[1] = -0.8; Vy[1] =  0.1;
    
#if 0
    srand(2);
#else
    srand( unsigned( fmod( HostClockTime()*1E3, 4*double(1<<30))));
#endif

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
#if 0
        case HOST_KEY_RETURN:
            VisibleDialog = NULL;
            break;
        case ' ':
            if( CultureBeginX<=DuckX && DuckX<CultureEndX ) 
                VisibleDialog = &WarnAwayFromCultureDialog;
            else
                // Fire airgun at Duck's feet, because firing airgun at surface generates wave that
                // mostly cancels itself.
                AirgunFire( DuckX, 8 );
            break;
        case 'a':
            IsAutoGainOn.toggleChecked();
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
        case 'g':
            ToggleShowGeology();
            break;
        case 'r':
            ToggleShowReservoir();
            break;
#endif
    }
}

static bool MouseIsDown;
static float MousePointX, MousePointY, MouseScalar;

static int HandleTolerance = 20;    // FIXME - scale to screen size

static void SelectHandle( int x, int y ) {
    SelectedHandle = HandleBufFind(x,y,HandleTolerance,Handle::maskAll); 
    if( SelectedHandle.kind==Handle::null ) {
        Handle h = HandleBufFind(x,y,1000,Handle::maskTail);   // FIXME - avoid hardcoding constant
        if( h.kind!=Handle::null ) {
            float sx = ViewScale*x + ViewOffsetX;
            float sy = ViewScale*y + ViewOffsetY;
            float p = std::fabs(EvaluatePotential(sx,sy));
            if( p>=0.5f ) {
                h.kind = Handle::tailHollow;
                SelectedHandle = h;
            } else {
                SelectedHandle = Handle();
            }
        }
    }
}

void GameMouseButtonDown( const NimblePoint& point, int k ) {
    using namespace Universe;
    MouseIsDown = true;
    float x = point.x;
    float y = point.y;
    MousePointX = ViewScale*x + ViewOffsetX;
    MousePointY = ViewScale*y + ViewOffsetY;
    MouseScalar = 0;
    SelectHandle(x, y);
    switch(SelectedHandle.kind) {
        case Handle::circle: {
            MouseScalar = Mass[SelectedHandle.index];
            break;
        }
        case Handle::tailHollow:
            MouseScalar = Charge[SelectedHandle.index];
            break;
        default:
            break;
    }
#if 0
    switch(k) {
        case 0:
            if( VisibleDialog ) 
                if( VisibleDialog->mouseDown(point) )
                    return;
            for( Clickable** c=ClickableSetEnd; --c>=ClickableSet;  ) 
                if( (*c)->mouseDown(point) )
                    return;
            break;
    }
#endif
}

void GameMouseButtonUp( const NimblePoint& point, int k ) {
    MouseIsDown = false;
#if 0
    if( VisibleDialog ) {
        switch( VisibleDialog->mouseUp(point) ) {
            case Dialog::update:
                if( VisibleDialog==&TheGeologyDialog )
                    CreateNewArea(/*recycle=*/true);
                else if( VisibleDialog==&TheShotDialog )
#if 0
                    // Show example of shot
                    GameKeyDown(' ');
#else
                    (void)0;
#endif
                break;
            case Dialog::hide:
                VisibleDialog = NULL;
                break;
            default:
                break;
        }
    }
    for( Clickable** c=ClickableSet; c!=ClickableSetEnd; ++c ) 
        (*c)->mouseUp(point);
#endif
}

static inline float Dist2( float x0, float y0, float x1, float y1 ) {
    float dx = x0-x1;
    float dy = y0-y1;
    return dx*dx + dy*dy;
}

void GameMouseMove( const NimblePoint& point ) {
    using namespace Universe;
    float x = point.x;
    float y = point.y;
    if( MouseIsDown ) {
        switch(SelectedHandle.kind) {
            case Handle::null: {
                ViewOffsetX = MousePointX - ViewScale*point.x;
                ViewOffsetY = MousePointY - ViewScale*point.y;
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
                float qx = MousePointX;
                float qy = MousePointY;
                // Current mouse position in universe
                float px = ViewScale*x + ViewOffsetX;
                float py = ViewScale*y + ViewOffsetY;
                float dot = (px-cx)*(MousePointX-cx) + (py-cy)*(MousePointY-cy);
                float ratio = std::sqrt(Dist2(px,py,cx,cy)/Dist2(qx,qy,cx,cy)) * (dot>=0 ? 1 : -1);
                if( SelectedHandle.kind==Handle::circle ) {
                    Mass[k] = ratio*MouseScalar;
                } else {
                    Charge[k] = ratio*MouseScalar;
                }
                break;
            }
        }
    } else {
        SelectHandle(x,y);
    }
#if 0
    if( VisibleDialog ) 
        VisibleDialog->mouseMove(point);
    for( Clickable** c=ClickableSet; c!=ClickableSetEnd; ++c )
        (*c)->mouseMove(point);
#endif
}
