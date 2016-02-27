#include "NimbleDraw.h"
#include <algorithm>
#include <cmath>
#include <limits>

static const float SqrtHalf = std::sqrt(0.5);

static void ClipOctant( float& ymin, float& ymax, float x0, float y0, float radius, float w, float h ) {
    float xmin = std::max(-x0, radius*SqrtHalf);
    float xmax = std::min(w - x0, radius);
    if( xmin>xmax )
        return;
    float ylower = std::max(-y0, std::sqrt(radius*radius - xmax*xmax));
    float yupper = std::min( h - y0, std::sqrt(radius*radius - xmin*xmin));
    if( ylower>yupper )
        return;
    // [min,ymax] |= [ylower,upper]
    ymin = std::min(ymin,ylower);
    ymax = std::max(ymax,yupper);
}

static void ClipQuadrant( float& ymin, float& ymax, float x0, float y0, float radius, float w, float h ) {
    ClipOctant(ymin, ymax, x0, y0, radius, w, h);
    ClipOctant(ymin, ymax, y0, x0, radius, h, w);
}

static void DrawPixel( const NimblePixMap& map, NimblePixel color, float xf, float yf ) {
    if( 0<=xf && xf<map.width() && 0<=yf && yf<map.height() ) {
        *(NimblePixel*)map.at(int(xf),int(yf)) = color;
    }
}

bool DrawCircle(const NimblePixMap& map, NimblePixel color, float x0, float y0, float r, bool dashed) {
    float w = map.width()-1;
    float h = map.height()-1;
    float ymin = std::numeric_limits<float>::infinity();
    float ymax = -ymin;
    ClipQuadrant(ymin, ymax, x0, y0, r, w, h);
    ClipQuadrant(ymin, ymax, w-x0, y0, r, w, h );
    ClipQuadrant(ymin, ymax, x0, h-y0, r, w, h );
    ClipQuadrant(ymin, ymax, w-x0, h-y0, r, w, h );
    if( ymin>ymax ) 
        return false;
    float y = ymin;
    float x = std::sqrt(r*r - ymin*ymin);
    float d = 2*(x*x + y*y - r*r) + (2*y+1) + (1-2*x);
    while(y <= ymax) {
        // FIXME - use lookup table indexed by quotients instead of trig
        if( !dashed || int(std::atan2(y,x)/(3.1415926535/4)*8+1)&2 ) {
            // FIXME - really cannot use octant symmetry since sub-pixel accuracy is desired
            DrawPixel(map, color, x + x0, y + y0); // Octant 1
            DrawPixel(map, color, y + x0, x + y0); // Octant 2
            DrawPixel(map, color, -x + x0, y + y0); // Octant 4
            DrawPixel(map, color, -y + x0, x + y0); // Octant 3
            DrawPixel(map, color, -x + x0, -y + y0); // Octant 5
            DrawPixel(map, color, -y + x0, -x + y0); // Octant 6
            DrawPixel(map, color, x + x0, -y + y0); // Octant 7
            DrawPixel(map, color, y + x0, -x + y0); // Octant 8
        }
        ++y;
        if( d<=0 ) {
            d += 4*y + 2;
        } else {
            --x;
            d += 4*(y-x) + 2;
        }
    }
    return true;
}