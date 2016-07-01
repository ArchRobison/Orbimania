#include "Universe.h"
#include "Clut.h"
#include "View.h"
#include "NimbleDraw.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include "AssertLib.h"

static const Universe::Float chargeScale = CLUT_SIZE/8;  // FIXME - have some kind of auto-scale?
                                                         // Value returned is scaled so that [-1,1] maps onto the Clut.
static inline float PotentialAt(size_t k, float x, float y) {
    using namespace Universe;
    Float dx = Sx[k]-x;
    Float dy = Sy[k]-y;
    return Charge[k]/std::sqrt(dx*dx+dy*dy);
}
    
Universe::Float EvaluatePotential(float x, float y) {
    using namespace Universe;
    Float p = 0;
    size_t n = NParticle;
    for(size_t k=0; k<n; ++k) {
        p += PotentialAt(k,x,y);
    }
    return p*(chargeScale*2/CLUT_SIZE);
}

// Particle parameters required for rendering.
struct Particle {
    float sx, sy, charge;
};

static const int PATCH_SIZE = 32;
static const float NEAR_RADIUS = 0.5;           // FIXME

static float NearX[N_PARTICLE_MAX];
static float NearY[N_PARTICLE_MAX];
static float NearCharge[N_PARTICLE_MAX];

//! Draw the potential field on the given map
void DrawPotentialFieldBilinear(const NimblePixMap& map) {
    using namespace Universe;
    int w = map.width();
    int h = map.height();
    size_t n = NParticle;
    // FIXME - deal with pixels near boundary that do not lie in a complete patch.
    for(int i0=0; i0+PATCH_SIZE<=h; i0+=PATCH_SIZE) {
        for(int j0=0; j0+PATCH_SIZE<=w; j0+=PATCH_SIZE) {
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
                if( std::sqrt(Dist2(sx,sy,xm,ym))*fabs(Charge[k]) <= NEAR_RADIUS ) {
                    // Use particle exactly
                    NearX[nearN] = sx;
                    NearY[nearN] = sy;
                    NearCharge[nearN] = Charge[k];
                    ++nearN;
                } else {
                    // Use bilinear interpolation
                    a00 += PotentialAt(k,x0, y0);
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
            for(int i=0; i<PATCH_SIZE; ++i) {
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
                    // Inner loop
                    for(int j=0; j<PATCH_SIZE; ++j) {
                        float dx = NearX[k]-x[j];
                        float r = std::sqrt(dx*dx+dy*dy);
                        p[j] += q/r;
                    }
                }
                using namespace Universe;
                Float lowerLimit = 0;
                Float upperLimit = CLUT_SIZE-1;
                NimblePixel* out = (NimblePixel*)map.at(j0, i0+i);
                for(int j=0; j<PATCH_SIZE; ++j) {
                    float v = p[j]*chargeScale + CLUT_SIZE/2 + 0.5;
                    if(v<=upperLimit); else v=upperLimit;
                    if(v>=lowerLimit); else v=lowerLimit;
                    out[j] = Clut[int(v)];
                }
            }
        }
    }
}

//! Draw the potential field on the given map
void DrawPotentialFieldPrecise(const NimblePixMap& map) {
    using namespace Universe;
    int w = map.width();
    int h = map.height();
    const size_t WIDTH_MAX = 4096;
    static Float x[WIDTH_MAX];
    static Float p[WIDTH_MAX];
    for(int j=0; j<w; ++j) {
        x[j] = ViewOffsetX + ViewScale*j;
    }
    size_t n = NParticle;
    // Work one row at a time to optimize cache usage
    for(int i=0; i<h; ++i) {
        Float y = ViewOffsetY + ViewScale*i;
        // Clear potential accumulator
        for(int j=0; j<w; ++j) {
            p[j] = 0;
        }
        // Do loop over particles as middle loop to hide latency of summation.
        for(size_t k=0; k<n; ++k) {
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
        NimblePixel* out = (NimblePixel*)map.at(0, i);
        for(int j=0; j<w; ++j) {
            float v = p[j]*chargeScale + CLUT_SIZE/2 + 0.5;
            if(v<=upperLimit); else v=upperLimit;
            if(v>=lowerLimit); else v=lowerLimit;
            out[j] = Clut[int(v)];
        }
    }
}

