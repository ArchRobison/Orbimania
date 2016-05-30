#include "Universe.h"
#include "Config.h"
#include "AssertLib.h"
#include "NimbleDraw.h"
#include "Clut.h"
#include "Handle.h"
#include "View.h"
#include <cmath>

void DrawArrow( const NimblePixMap& map, NimblePixel color, float x0, float y0, float x1, float y1 );
bool DrawDot( const NimblePixMap& map, NimblePixel color, float x0, float y0, float r );
bool DrawCircle( const NimblePixMap& map, NimblePixel color, float x0, float y0, float r, bool dashed);

Handle SelectedHandle;
static NimblePixel UnselectedHandleColor( NimbleColor(128).pixel());
static NimblePixel SelectedHandleColor( NimbleColor(255,0,255).pixel());
static NimblePixel ArrowColor( NimbleColor(128).pixel());

static void DrawHandle( const NimblePixMap& map, float x, float y, float r, Handle::kindType kind, size_t k ) {
    NimblePixel color = SelectedHandle.match(kind,k) ? SelectedHandleColor : UnselectedHandleColor;
    if( r==0 ) {
        if( DrawDot(map, color, x, y, 3)) {
            HandleBufAdd(x, y, 0, Handle(kind, k));
        } 
    } else {
        bool dashed = false;
        if(r<0) {
            dashed = true;
            r = -r;
        }
        if(DrawCircle(map, color, x, y, r, dashed)) {
            HandleBufAdd(x, y, r, Handle(kind, k));
        }
    }
}

void DrawMarkup( const NimblePixMap& map ) {
    HandleBufClear();
    using namespace Universe;
    size_t n = NParticle;
    float scale = 1/ViewScale;
    float tipScale = 1/ViewVelocityScale;        
    StateVar x0, y0, x1, y1;
    for( size_t k=0; k<n; ++k ) {
         x0[k] = (Sx[k] - ViewOffsetX) * scale;
         y0[k] = (Sy[k] - ViewOffsetY) * scale;
         x1[k] = x0[k] + (Vx[k] * tipScale);
         y1[k] = y0[k] + (Vy[k] * tipScale);
    }
    for(size_t k=0; k<n; ++k) {
        DrawHandle(map, x0[k], y0[k], Mass[k]/ViewMassScale, Handle::circle, k);
    }
    for( size_t k=0; k<n; ++k ) {
        DrawArrow(map, ArrowColor, x0[k], y0[k], x1[k], y1[k] );
    }
    for(size_t k=0; k<n; ++k) {
        if( SelectedHandle.match(Handle::tailHollow,k)) {
            DrawHandle(map, x0[k], y0[k], 5, Handle::tailHollow, k); 
        } else {
            DrawHandle(map, x0[k], y0[k], 0, Handle::tailFull, k); 
        }
        DrawHandle(map, x1[k], y1[k], 0, Handle::head, k); 
    }
}
