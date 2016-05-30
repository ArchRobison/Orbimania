#include <cstdio>
#include "NimbleDraw.h"
#include "Config.h"

namespace Universe {

typedef float Float;

typedef Float StateVar[N_PARTICLE_MAX];

extern StateVar
    Mass,   // mass of particle
    Charge, // monopole charge of particle
    Sx,	 // X coordinate at current time step.
    Sy,	 // Y coordinate at current time step.
    Vx,	 // X velocity at previous half time step.
    Vy;	 // Y velocity at previous falf time step.

extern size_t NParticle;
extern Float DeltaT;

}

Universe::Float EvaluatePotential(float x, float y);
void DrawPotentialFieldSlow( const NimblePixMap& map );
void DrawPotentialFieldFast(const NimblePixMap& map);
void DrawMarkup( const NimblePixMap& map );
void AdvanceUniverseOneTimeStep(); // in TimeStep.cpp

// Return square of x
template<typename T>
static inline T Square(T x) {
    return x*x;
}

// Return square of distance from (x0,y0) to (x1,y1)
template<typename T>
static inline T Dist2(T x0, T y0, T x1, T y1) {
    T dx = x0-x1;
    T dy = y0-y1;
    return dx*dx + dy*dy;
}
