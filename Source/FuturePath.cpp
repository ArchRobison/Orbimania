#include "NimbleDraw.h"
#include "Universe.h"
#include "View.h"
#include <cstring>
#include <cstdint>

using namespace Universe;

static StateVar SaveSx, SaveSy, SaveVx, SaveVy;

static inline void Copy( StateVar& dst, StateVar& src ) {
    std::memcpy(dst,src,NParticle*sizeof(dst[0]));
}

void DrawFuturePaths(NimblePixMap& map) {
    Copy(SaveSx, Sx);
    Copy(SaveSy, Sy);
    Copy(SaveVx, Vx);
    Copy(SaveVy, Vy);
    size_t n = NParticle;
    unsigned h = map.height();
    unsigned w = map.width();
    for( size_t t=0; t<64; ++t ) {
        for( int j=0; j<16; ++j )
            AdvanceUniverseOneTimeStep();
        for( size_t k=0; k<NParticle; ++k ) {
            float x = (Sx[k] - ViewOffsetX)/ViewScale;
            float y = (Sy[k] - ViewOffsetY)/ViewScale;
            if( 0<=x && x<=w-1 && 0<=y && y<=h-1 )
                *(uint32_t*)map.at(x,y) ^= 0xFFFFFF;
        }
    }
    Copy(Sx, SaveSx);
    Copy(Sy, SaveSy);
    Copy(Vx, SaveVx);
    Copy(Vy, SaveVy);
}