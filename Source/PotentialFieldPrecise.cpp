#include "Clut.h"
#include "PotentialField.h"
#include "View.h"

Universe::Float ChargeScale = CLUT_SIZE/8;  // FIXME - have some kind of auto-scale?
                                            // Value returned is scaled so that [-1,1] maps onto the Clut.

Universe::Float EvaluatePotential(float x, float y) {
    Universe::Float p = 0;
    size_t n = Universe::NParticle;
    for(size_t k=0; k<n; ++k) {
        p += PotentialAt(k, x, y);
    }
    return p*(ChargeScale*2/CLUT_SIZE);
}

//! Draw the potential field on the given map
void DrawPotentialFieldPrecise(const NimblePixMap& map) {
    using namespace Universe;
    int w = map.width();
    int h = map.height();
    static Float x[DISPLAY_WIDTH_MAX];
    static Float p[DISPLAY_WIDTH_MAX];
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
        DrawPotentialRow((NimblePixel*)map.at(0, i), p, w );
    }
}

