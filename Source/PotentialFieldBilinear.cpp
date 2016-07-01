#include "Universe.h"
#include "Clut.h"
#include "View.h"
#include "NimbleDraw.h"
#include "PotentialField.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include "AssertLib.h"

static const int PATCH_SIZE = 32;           // Use multiple of # of floats that fit in SIMD register
static const float NEAR_RADIUS = 0.5;       // FIXME

static float NearX[N_PARTICLE_MAX];
static float NearY[N_PARTICLE_MAX];
static float NearCharge[N_PARTICLE_MAX];

//! Draw the potential field on the given map
void DrawPotentialFieldBilinear(const NimblePixMap& map) {
    using namespace Universe;
    int w = map.width();
    int h = map.height();
    size_t n = NParticle;
    float cutoffRadius = 8*ViewScale*PATCH_SIZE;
    // FIXME - deal with pixels near boundary that do not lie in a complete patch.
    for(int i0=0; i0<h; i0+=PATCH_SIZE) {
        int iSize = std::min(PATCH_SIZE,h-i0);
        for(int j0=0; j0<w; j0+=PATCH_SIZE) {
            float a00 = 0, a01 = 0, a10 = 0, a11 = 0;
            float y0 = ViewOffsetY + ViewScale*i0;
            float x0 = ViewOffsetX + ViewScale*j0;
            float y1 = ViewOffsetY + ViewScale*(i0 + PATCH_SIZE);
            float x1 = ViewOffsetX + ViewScale*(j0 + PATCH_SIZE);
            float xm = 0.5f*(x0+x1);
            float ym = 0.5f*(y0+y1);
            size_t nearN = 0;
            for(size_t k=0; k<n; ++k) {
                float sx = Sx[k];
                float sy = Sy[k];
                if( std::sqrt(Dist2(sx,sy,xm,ym)) <= cutoffRadius ) {
                    // Use particle exactly
                    NearX[nearN] = sx;
                    NearY[nearN] = sy;
                    NearCharge[nearN] = Charge[k];
                    ++nearN;
                } else {
                    // Use bilinear interpolation
                    a00 += PotentialAt(k, x0, y0);
                    a01 += PotentialAt(k, x0, y1);
                    a10 += PotentialAt(k, x1, y0);
                    a11 += PotentialAt(k, x1, y1);
                }
            }
            float x[PATCH_SIZE], p[PATCH_SIZE];
            for(int j=0; j<PATCH_SIZE; ++j) {
                x[j] = ViewOffsetX + ViewScale*(j0+j);
            }
            // Work one row at a time to optimize cache usage
            // j-loops really only have to do jSize iterations, but do PATCH_SIZE anyway on theory
            // that it avoids remainder loops.
            int jSize = std::min(PATCH_SIZE, w-j0);
            for(int i=0; i<iSize; ++i) {
                // Set potential accumulator to bilinear interpolation values
                float fy = i*(1.0f/PATCH_SIZE);
                for(int j=0; j<PATCH_SIZE; ++j) {
                    float fx = j*(1.0f/PATCH_SIZE);
                    p[j] = a00*(1-fy)*(1-fx) + a01*fy*(1-fx) + a10*(1-fy)*fx + a11*fy*fx;
                }
                // Loop over particles is middle loop to hide latency of summation.
                float y = ViewOffsetY + ViewScale*(i0+i);
                for(size_t k=0; k<nearN; ++k) {
                    float dy = NearY[k]-y;
                    float q = NearCharge[k];
                    // Inner loop. 
                    for(int j=0; j<PATCH_SIZE; ++j) {
                        float dx = NearX[k]-x[j];
                        float r = std::sqrt(dx*dx+dy*dy);
                        p[j] += q/r;
                    }
                }
                DrawPotentialRow((NimblePixel*)map.at(j0, i0+i), p, jSize);
            }
        }
    }
}