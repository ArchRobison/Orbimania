#include "Universe.h"

namespace Universe {

StateVar
    Mass,   // mass of particle
    Charge, // monopole charge of particle
    Sx,	    // X coordinate at current time step.
    Sy,	    // Y coordinate at current time step.
    Vx,	    // X velocity at previous half time step.
    Vy;	    // Y velocity at previous falf time step.

size_t NParticle;
float DeltaT=.005;

// Erase kth particle
void EraseParticle( size_t k ) {
    size_t n = --NParticle;
    for(size_t j=k; j<n; ++j) {
        Mass[j] = Mass[j+1];
        Charge[j] = Charge[j+1];
        Sx[j] = Sx[j+1];
        Sy[j] = Sy[j+1];
        Vx[j] = Vx[j+1];
        Vy[j] = Vy[j+1];
    }
}

//! Reset universe so that center of mass is at (0,0) and center of momentum is stationary with respect to coordinate system.
void Recenter() {
    Float cx = 0, cy = 0;   // Accumulates moment computing center of momentum
    Float px = 0, py = 0;   // Accumulates total momentum
    Float m = 0;
    size_t n = NParticle;
    for( size_t k=0; k<n; ++k ) {
        px += Mass[k]*Vx[k];
        py += Mass[k]*Vy[k];
        cx += Mass[k]*Sx[k];
        cy += Mass[k]*Sy[k];
        m += Mass[k];
    }
    if( m==0 )
        // Center of mass is undefined
        return;
    Float vx = px/m;
    Float vy = py/m;
    Float sx = cx/m;
    Float sy = cy/m;
    for( size_t k=0; k<n; ++k ) {
        Vx[k] -= vx;
        Vy[k] -= vy;
        Sx[k] -= sx;
        Sy[k] -= sy;
    }
}

} // namespace Universe