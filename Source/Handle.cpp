#include "Handle.h"
#include "Config.h"
#include "AssertLib.h"
#include "PotentialField.h"
#include "Universe.h"
#include "View.h"
#include <cmath>

Handle SelectedHandle;

// 3 = tail + head + circle
const size_t N_HANDLE_MAX = 3*N_PARTICLE_MAX;

static Handle HandleBuf[N_HANDLE_MAX];
static float HandleX[N_HANDLE_MAX];
static float HandleY[N_HANDLE_MAX];
static float HandleR[N_HANDLE_MAX];

static size_t HandleBufSize;

void HandleBufClear() {
    HandleBufSize = 0;
}

void HandleBufAdd( float x, float y, float r, const Handle& h ) {
    size_t i = HandleBufSize++;
    Assert(i < sizeof(HandleBuf)/sizeof(HandleBuf[0]));
    HandleBuf[i] = h;
    HandleX[i] = x;
    HandleY[i] = y;
    HandleR[i] = r;
}

static Handle HandleBufFind( float x, float y, float maxDist, unsigned mask ) {
    Handle best;
    float bestDist = maxDist;
    for( size_t i=0; i<HandleBufSize; ++i ) {
        if(1<<HandleBuf[i].kind & mask) {
            float dx = x - HandleX[i];
            float dy = y - HandleY[i];
            float d = std::fabs(std::sqrt(dx*dx + dy*dy) - HandleR[i]);
            if(d<=bestDist) {
                best = HandleBuf[i];
                bestDist = d;
            }
        }
    }
    return best;
}

static int HandleTolerance = 15;    // FIXME - scale to screen size

// Set SelectedHandle to handle selected by cursor point (x,y)
void SelectHandle(int x, int y) {
    // Check if user is trying to select handle directly by pointing to it.
    SelectedHandle = HandleBufFind(x, y, HandleTolerance, Handle::maskAll);
    switch(SelectedHandle.kind) {
        case Handle::null: {
            // No handle is close.  But maybe user is trying to change charge strength of closest charge.
            Handle h = HandleBufFind(x, y, 1000, Handle::maskTail);   // FIXME - avoid hardcoding constant
            if(h.kind!=Handle::null) {
                float sx = ViewScale*x + ViewOffsetX;
                float sy = ViewScale*y + ViewOffsetY;
                float p = std::fabs(EvaluatePotential(sx, sy));
                if(p>=0.5f) {
                    h.kind = Handle::tailHollow;
                    SelectedHandle = h;
                } else {
                    SelectedHandle = Handle();
                }
            }
            break;
        }
        case Handle::tailHollow:
            // Handle was not close before now, but now is close.
            SelectedHandle.kind = Handle::tailFull;
            break;
    }
}

// Update handles after handle k has been erased.
void UpdateHandlesAfterErasure(size_t k) {
    Assert( SelectedHandle.index!=k );
    size_t n = HandleBufSize;
    size_t i=0;
    for( size_t j=0; j<n; ++j  ) {
        if( HandleBuf[j].index!=k ) {
            HandleX[i] = HandleX[j];
            HandleY[i] = HandleY[j];
            HandleR[i] = HandleR[j];
            HandleBuf[i] = HandleBuf[j];
            ++i;
        }
    }
    HandleBufSize = i;
}

