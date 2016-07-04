#include "Universe.h"
#include <cmath>

//	Algorithm uses method described in:
//
//		Completely Conservative, Covariant Numerical Methodology
//  	D. Greenspan
//  	Computers Math Applic. Vol 29(4), pp. 37-43.
//
//  but with Newton's method replaced by fixed-point solver.

using namespace Universe;

// Estimated values for next timestep
static StateVar Sx_, Sy_, Vx_, Vy_ ;
static StateVar Fx, Fy;

inline float Dist( size_t i, size_t j, StateVar sx, StateVar sy ) {
    float dx = sx[i] - sx[j];
    float dy = sy[i] - sy[j];
    return std::sqrt(dx*dx+dy*dy);
}

static void ComputeForce() {
    size_t n = NParticle;
    for( size_t i=0; i<n; ++i ) {
        Fx[i] = 0;
        Fy[i] = 0;
    }
    for( size_t i=0; i<n-1; ++i ) {
        for( size_t j=i+1; j<n; ++j ) {
            float d = Dist(i,j,Sx,Sy);
            float d_ = Dist(i,j,Sx_,Sy_);
            // Formula for f is Greenspan term with phi = 1/d and simplifed via (1/d_ - 1/d)/(d_ - d) = -1/(d_*d)
            float f = -Charge[i]*Charge[j] / (d_*d);            // FIXME - sign might need to be flipped
            float ux = ((Sx_[i]+Sx[i])-(Sx_[j]+Sx[j])) / (d+d_);
            float uy = ((Sy_[i]+Sy[i])-(Sy_[j]+Sy[j])) / (d+d_);
            Fx[i] -= f*ux;
            Fy[i] -= f*uy;
            Fx[j] += f*ux;
            Fy[j] += f*uy;
        }
    }
}

static float UpdateNextPosition() {
    size_t n = NParticle;
    float error = 0;
    float h = 0.5f*DeltaT;              // h = half of deltaT
    for( size_t i=0; i<n; i++ ) {
        // Compute position using average velocity
        float sx_ = Sx[i] + h*(Vx[i] + Vx_[i]);
        float sy_ = Sy[i] + h*(Vy[i] + Vy_[i]);
        // Compare with previously computed future position
        error += Dist2(sx_, sy_, Sx_[i], Sy_[i]);
        Sx_[i] = sx_;
        Sy_[i] = sy_;
    }
    return error;
}

static float UpdateNextVelocity() {
    size_t n = NParticle;
    float error = 0;
    // Compute velocities
    for( size_t i=0; i<n; i++ ) {
    	// Compute linear velocity from force.
        float vx_ = Vx[i] + (DeltaT/Mass[i])*Fx[i]; 
        float vy_ = Vy[i] + (DeltaT/Mass[i])*Fy[i];
        error += Dist2(vx_, vy_, Vx_[i], Vy_[i]); 
        Vx_[i] = vx_;
        Vy_[i] = vy_;
    }
    return error;
}

void AdvanceUniverseOneTimeStep() {
    size_t n = NParticle;
    // Set future position assuming constant velocity.
    for( size_t i=0; i<n; i++ ) {
        Vx_[i] = Vx[i];
        Vy_[i] = Vy[i];
    }
    // Do up to 16 iterations of fixed-point root finder.
    float olderr = 0;
    for( int k=0; k<16; k++ ) {
	    float errp = UpdateNextPosition();
	    ComputeForce();
	    float errv = UpdateNextVelocity();
        // Adding square-errors with different dimensional units is questionable
	    float err = errp+errv;
        if( err==0 )
            break;
 	    if( k>0 && err>olderr ) {
	        char * ouch = "ouch";
	    }
        olderr = err;
	}
    // Advance one time step.
    for( size_t i=0; i<n; i++ ) {
        Sx[i] = Sx_[i]; 
        Sy[i] = Sy_[i]; 
        Vx[i] = Vx_[i]; 
        Vy[i] = Vy_[i]; 
    }
}
