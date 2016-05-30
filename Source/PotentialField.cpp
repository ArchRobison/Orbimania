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
Universe::Float EvaluatePotential(float x, float y) {
    using namespace Universe;
    Float p = 0;
    size_t n = NParticle;
    for(size_t k=0; k<n; ++k) {
        Float dx = Sx[k]-x;
        Float dy = Sy[k]-y;
        p += Charge[k]/std::sqrt(dx*dx+dy*dy);
    }
    return p*(chargeScale*2/CLUT_SIZE);
}

struct Node {
    float sx, sy, charge;   // Far-field parameters if non-leaf, particle if leaf
    float r;                // Width of node. 0 for leaf.
    Node* child[4];         // null for empty node
};

static const size_t NodeArraySize = 2*N_PARTICLE_MAX;
static Node NodeArray[NodeArraySize];
static Node *NodeEnd;

struct Particle {
    float sx, sy, charge;
};

static Particle ParticleArray[N_PARTICLE_MAX];

static Particle* PartitionByX(Particle* first, Particle* last, float xcenter ) {
    return std::partition(first, last, [xcenter](const Particle& p) {return p.sx<xcenter;});
}

static Particle* PartitionByY(Particle* first, Particle* last, float ycenter) {
    return std::partition(first, last, [ycenter](const Particle& p) {return p.sy<ycenter; });
}

static Node* BuildQuadTreeRec( Particle* first, Particle* last ) {
    if( first==last )
        return nullptr;
    Assert(NodeEnd < &NodeArray[NodeArraySize]);
    Node* n = NodeEnd++;
    if( last==first+1 ) {
        // Node has exactly one particle
        n->sx = first->sx;
        n->sy = first->sy;
        n->charge = first->charge;
        n->r = 0;
        for( int k=0; k<4; ++k )
            n->child[k] = nullptr;
        return n;
    };
    // Find bounding square
    float xmin = first->sx, ymin = first->sy, xmax = first->sx, ymax = first->sy;
    for( auto i=first+1; i!=last; ++i ) {
        if( i->sx<xmin ) xmin = i->sx;
        else if( i->sx>xmax ) xmax = i->sx;
        if( i->sy<ymin ) ymin = i->sy;
        else if(i->sy>ymax) ymax = i->sy;
    }
    float xcenter = 0.5*(xmin+xmax);
    float ycenter = 0.5*(ymin+ymax);
    n->r = std::max(xmax-xcenter, ymax-ycenter);
    float sx = 0, sy = 0, charge = 0;
    float glomDiameter = 0.000001f;      // FIXME - compute glomDiameter based on pixel size
    if( ymax-ymin < glomDiameter && xmax-xmin < glomDiameter) {
        // square is tiny - just glom everything
        for( auto i=first; i!=last; ++i ) {
            sx += i->sx * charge;
            sy += i->sy * charge;
            charge += i->charge;
        }
        for(int k=0; k<4; ++k)
            n->child[k] = nullptr;
    } else {
        // Partition into quadrants
        Particle* p2 = PartitionByY(first, last, ycenter);
        Particle* p1 = PartitionByX(first, p2, xcenter);
        Particle* p3 = PartitionByX(p2, last, xcenter);
        n->child[0] = BuildQuadTreeRec(first, p1);
        n->child[1] = BuildQuadTreeRec(p1, p2);
        n->child[2] = BuildQuadTreeRec(p2, p3);
        n->child[3] = BuildQuadTreeRec(p3, last);
        for(int k=0; k<4; ++k)
            if(Node* m = n->child[k]) {
                sx += m->sx*m->charge;
                sy += m->sy*m->charge;
                charge += m->charge;
            }
    }
    // Compute summary information
    n->charge = charge;
    if( charge!=0 ) {
        n->sx = sx / charge;
        n->sy = sy / charge;
    } else {
        // Avoid division by zero
        n->sx = xcenter;
        n->sy = ycenter;
    }
    return n;
}

static Node* BuildQuadTree() {
    using namespace Universe;
    size_t n = NParticle;
    // Copy particle (x,y,charge) information to array of Particle
    for( size_t i=0; i<n; ++i ) {
        Particle* p = &ParticleArray[i];
        p->sx = Sx[i];
        p->sy = Sy[i];
        p->charge = Charge[i];
    }
    NodeEnd = NodeArray;
    return BuildQuadTreeRec(ParticleArray, ParticleArray+n);
}

// A set of charges drawn from a quadtree.
class QuadTreeSlice {
    // Always single-precision, regardless of what Universe::StateVar is.
    typedef float StateVar[N_PARTICLE_MAX];
    size_t nParticle;
    StateVar sx, sy, charge;
public:
    void clear() {nParticle=0;}
    // Get charges (or summary of charges) from given tree rooted at node.
    // (x,y) is center of patch, r is "radius" of patch (which is square)
    void fill( const Node* node, float x, float y, float r);
    void drawPatch(const NimblePixMap& map, int i0, int j0);
    size_t size() const {return nParticle;}
};

void QuadTreeSlice::fill(const Node* node, float x, float y, float r) {
    float threshhold = 0.25;
    // FIXME -avoid need for sqrt
    if( node->r==0 || (node->r+r)/sqrt(Dist2(x, y, node->sx, node->sy)) < threshhold ) {
        // Node is exact, or so far away that that its summary can be used.
        size_t n = nParticle++;
        sx[n] = node->sx;
        sy[n] = node->sy;
        charge[n] = node->charge;
    } else {
        for(int k=0; k<4; ++k)
            if(auto m = node->child[k])
                fill(m, x, y, r);
    }
}

static const int PATCH_SIZE = 32;

void QuadTreeSlice::drawPatch(const NimblePixMap& map, int i0, int j0) {
    size_t n = nParticle;
    const float* sx = this->sx;
    const float* sy = this->sy;
    const float* charge = this->charge;
    float p[PATCH_SIZE];
    float x[PATCH_SIZE];
    for(int j=0; j<PATCH_SIZE; ++j) {
        x[j] = ViewOffsetX + ViewScale*(j0+j);
    }

    // Work one row at a time to optimize cache usage
    for(int i=0; i<PATCH_SIZE; ++i) {
        // Clear potential accumulator
        for(int j=0; j<PATCH_SIZE; ++j) {
            p[j] = 0;
        }
        // Loop over particles is middle loop to hide latency of summation.
        float y = ViewOffsetY + ViewScale*(i0+i);
        for(size_t k=0; k<n; ++k) {
            float dy = sy[k]-y;
            float q = charge[k];
            // Inner loop
            for(int j=0; j<PATCH_SIZE; ++j) {
                float dx = sx[k]-x[j];
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
#if 0
    // Mark upper left corner of patch
    *(NimblePixel*)map.at(j0, i0) = NimblePixel(0xFFFFFF);
#endif
}

#define DUMP_SLICE_AVG 0

void DrawPotentialFieldFast(const NimblePixMap& map) {
#if DUMP_SLICE_AVG
    int total = 0;
    int count = 0;
#endif
    QuadTreeSlice slice;
    using namespace Universe;
    int w = map.width();
    int h = map.height();
    Node* root = BuildQuadTree();
    // FIXME - deal with pixels near boundary that do not lie in a complete patch.
    for(int i0=0; i0+PATCH_SIZE<=h; i0+=PATCH_SIZE) {
        for(int j0=0; j0+PATCH_SIZE<=w; j0+=PATCH_SIZE) {
            float yc = ViewOffsetY + ViewScale*(i0 + 0.5*PATCH_SIZE);
            float xc = ViewOffsetX + ViewScale*(j0 + 0.5*PATCH_SIZE);
            slice.clear();
            slice.fill(root, xc, yc, ViewScale*0.5*PATCH_SIZE);
#if DUMP_SLICE_AVG
            total += slice.size();
            count += 1;
#endif
            slice.drawPatch(map, i0, j0);
        }
    }
#if DUMP_SLICE_AVG
    printf("%g\n",double(total)/count);
#endif
}

//! Draw the potential field on the given map
void DrawPotentialFieldSlow(const NimblePixMap& map) {
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

