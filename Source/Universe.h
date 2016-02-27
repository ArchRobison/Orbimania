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
void DrawPotentialField( const NimblePixMap& map );  
void DrawMarkup( const NimblePixMap& map );
void AdvanceUniverseOneTimeStep(); // in TimeStep.cpp
