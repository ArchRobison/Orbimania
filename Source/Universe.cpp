#include "Universe.h"

namespace Universe {

StateVar
    Mass,   // mass of particle
    Charge, // monopole charge of particle
    Sx,	 // X coordinate at current time step.
    Sy,	 // Y coordinate at current time step.
    Vx,	 // X velocity at previous half time step.
    Vy;	 // Y velocity at previous falf time step.

size_t NParticle;
float DeltaT=.005;

}