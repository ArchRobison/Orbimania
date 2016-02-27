#include "Universe.h"
#include "Config.h"
#include "AssertLib.h"
#include "NimbleDraw.h"
#include "Clut.h"
#include "Handle.h"
#include "View.h"
#include <cmath>

static const Universe::Float chargeScale = CLUT_SIZE/8;  // FIXME - have some kind of auto-scale?

// Value returned is scaled so that [-1,1] maps onto the Clut.
Universe::Float EvaluatePotential(float x, float y) {
    using namespace Universe;
    Float p = 0;
    size_t n = NParticle;
    for( size_t k=0; k<n; ++k ) {
        Float dx = Sx[k]-x;
        Float dy = Sy[k]-y;
        p += Charge[k]/std::sqrt(dx*dx+dy*dy);
    }
    return p*(chargeScale*2/CLUT_SIZE);
}

//! Draw the potential field on the given map
void DrawPotentialField( const NimblePixMap& map ) {
    using namespace Universe;
    int w = map.width();
    int h = map.height();
    const size_t WIDTH_MAX = 4096;
    static Float x[WIDTH_MAX];
    static Float p[WIDTH_MAX];
    for( int j=0; j<w; ++j ) {
        x[j] = ViewOffsetX + ViewScale*j;
    }
    size_t n = NParticle;
    // Work one row at a time to optimize cache usage
    for( int i=0; i<h; ++i ) {
        Float y = ViewOffsetY + ViewScale*i;
        // Clear potential accumulator
        for( int j=0; j<w; ++j ) {
            p[j] = 0;
        }
        // Do loop over particles as middle loop to hide latency of summation.
        for( size_t k=0; k<n; ++k ) {
            float dy = Sy[k]-y;
            float q = Charge[k];
            // Inner loop
            // FIXME - vectorize via recprocal approximation and Newton-Raphson
            for(int j=0; j<w; ++j) {
                Float dx = Sx[k]-x[j];
                Float r = std::sqrt(dx*dx+dy*dy);
                p[j] += q/r;
            }
        }
        Float lowerLimit = 0;
        Float upperLimit = CLUT_SIZE-1;
        NimblePixel* out = (NimblePixel*)map.at(0,i);
        for( int j=0; j<w; ++j ) {
            float v = p[j]*chargeScale + CLUT_SIZE/2 + 0.5;
            if( v<=upperLimit ); else v=upperLimit;
            if( v>=lowerLimit ); else v=lowerLimit;
            out[j] = Clut[int(v)];
        }
    }
} 

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
