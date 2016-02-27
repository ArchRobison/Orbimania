#include "NimbleDraw.h"
#include <cmath>
#include <algorithm>

// FIXME - optimize clipping, or not wortht the trouble?

template<bool Transpose>
static void DrawLineAux( const NimblePixMap& map, NimblePixel color, float x0, float y0, float x1, float y1 ) {
    int w = (Transpose ? map.height() : map.width()) - 1;
    int h = (Transpose ? map.width() : map.height()) - 1;
    int xlower = x0 < 0 ? 0 : int(x0);
    int xupper = x1 > w ? w : int(x1);
    if( xlower<=xupper ) {
        float dx = x1-x0;
        float dy = y1-y0;
        float slope = dx==0 ? 0 : dy/dx;
        float intercept = y0 - slope * x0;
        int x = xlower;
        do {
            float y = intercept+slope*x;
            if(0<=y && y<=h)
                *(NimblePixel*)(Transpose? map.at(y, x) : map.at(x, y)) = color;
        } while(++x<=xupper);
    }
}

void DrawLine( const NimblePixMap& map, NimblePixel color, float x0, float y0, float x1, float y1 ) {
    int w = map.width()-1;
    int h = map.height()-1;
    float dx = x1-x0;
    float dy = y1-y0;
    if( std::fabs(dy)<=std::fabs(dx)) {
        if( x0<=x1 )
            DrawLineAux<false>(map,color,x0,y0,x1,y1);
        else
            DrawLineAux<false>(map,color,x1,y1,x0,y0);
    } else {
        if( y0<=y1 )
            DrawLineAux<true>(map,color,y0,x0,y1,x1);
        else
            DrawLineAux<true>(map,color,y1,x1,y0,x0);
    }
}

const float Angle = 0.78539816339;
const float Size = 0.125;

const float Alpha = Size*std::cos(Angle);
const float Beta = Size*std::sin(Angle);

void DrawArrow( const NimblePixMap& map, NimblePixel color, float x0, float y0, float x1, float y1 ) {
    // Draw line segment
    DrawLine(map,color,x0,y0,x1,y1);
    // Draw tip
    float dx = x0-x1;
    float dy = y0-y1;
    DrawLine(map, color, x1, y1, x1 + dx*Alpha + dy*Beta, y1 - dx*Beta + dy*Alpha);
    DrawLine(map, color, x1, y1, x1 + dx*Alpha - dy*Beta, y1 + dx*Beta + dy*Alpha);
}

bool DrawDot( const NimblePixMap& map, NimblePixel color, float x0, float y0, float r ) {
    int w = map.width()-1;
    int h = map.height()-1;

    int xlower = x0-r;
    if( xlower<0 ) xlower=0;
    int xupper = x0+r;
    if( xupper>w ) xupper=w;
    if( xupper<xlower )
        return false;

    int ylower = y0-r;
    if( ylower<0 ) ylower=0;
    int yupper = y0+r;
    if( yupper>h ) yupper=h;
    if( yupper<ylower )
        return false;

    for( int y=ylower; y<=yupper; ++y ) {
        for( int x=xlower; x<=xupper; ++x ) {
            float dx = x-x0;
            float dy = y-y0;
            if( dx*dx + dy*dy < r*r ) {
                *(NimblePixel*)map.at(x, y) = color;
            }
        }
    }
    return true;
}