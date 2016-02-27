#include "Handle.h"
#include "Config.h"
#include "AssertLib.h"
#include <cmath>

const size_t N_HANDLE_MAX = 2*N_PARTICLE_MAX;

// 2 = tail + head
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

Handle HandleBufFind( float x, float y, float maxDist, unsigned mask ) {
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