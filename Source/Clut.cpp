#include "Clut.h"
#include "ColorMatrix.h"

NimblePixel Clut[CLUT_SIZE];

ColorMatrix GuideColors("ColorLookupTable");

void InitializeClut() {
    int w = GuideColors.width();
    int h = GuideColors.height();

    float t[CLUT_SIZE+1];
    Assert(w%2==1);
    Assert(h%2==1);
    for( int j=0; j<=CLUT_SIZE/2; ++j ) {
        // u is normalized amplitude in range [-1,1]
        float v = j*(2.0f/CLUT_SIZE)*(w/2);
        t[CLUT_SIZE/2+j] = v;
        t[CLUT_SIZE/2-j] = -v;
    }

    const NimbleColor* colorScale0 = GuideColors[0]+w/2;
    for( int j=0; j<CLUT_SIZE; ++j ) {
        int k = int(t[j]);
        float residue = t[j]-k;
        Assert( k<GuideColors.width()/2 || k==GuideColors.width()/2 && residue==0 );
        NimbleColor c0 = colorScale0[k];
        if( residue>0 ) {
            c0.mix(colorScale0[k+1],residue);
        }
        if( residue<0 ) {
            c0.mix(colorScale0[k-1],-residue);
        }
        Clut[j] = c0.pixel(); 
    }
}