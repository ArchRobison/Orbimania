#include "View.h"
#include "AssertLib.h"
#include <cmath>

static float DefaultViewScale = 1.0f/768;
static float DefaultMassScale = 1.0f/100;
static float DefaultChargeScale = 1.0f/100;

// x in universe = ViewScale*(pixel for x) + ViewOffsetX
float ViewScale = DefaultViewScale; // FIXME - set per screen size
float ViewOffsetX = 0;
float ViewOffsetY = 0;

// vx in universe = ViewVelocityScale*(pixel diff for x)
float ViewVelocityScale = 4*ViewScale;

// mass in universe =  ViewMassScale*(circle radius in pixels)
float ViewMassScale = DefaultMassScale;

// charge in universe =  ViewChargeScale*(circle radius in pixels)
float ViewChargeScale = DefaultChargeScale;

int ZoomLevel;

const int MaxLevel = 20;

void SetZoom(int level, int centerX, int centerY) {
    if( level>=MaxLevel ) level=MaxLevel;
    if( level<-MaxLevel ) level=-MaxLevel;
    float oldScale = ViewScale;
    ZoomLevel = level;
    ViewScale = std::pow(std::sqrt(0.5), ZoomLevel)*DefaultViewScale;
    ViewOffsetX += centerX*(oldScale-ViewScale);
    ViewOffsetY += centerY*(oldScale-ViewScale);
    ViewVelocityScale = 4*ViewScale;
    ViewMassScale = ViewScale*(DefaultMassScale/DefaultViewScale);
}

void RecenterView(int centerX, int centerY) {
    ViewOffsetX = -centerX*ViewScale;
    ViewOffsetY = -centerY*ViewScale;
}