/* Copyright 1996-2016 Arch D. Robison

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma once
#ifndef PotentialField_H
#define PotentialField_H

#include <cmath>
#include "Universe.h"
#include "NimbleDraw.h"

Universe::Float EvaluatePotential(float x, float y);
void DrawPotentialFieldPrecise(const NimblePixMap& map);
void DrawPotentialFieldBarnesHut(const NimblePixMap& map);
void DrawPotentialFieldBilinear(const NimblePixMap& map);

static inline float PotentialAt(size_t k, float x, float y) {
    using namespace Universe;
    Float dx = Sx[k]-x;
    Float dy = Sy[k]-y;
    return Charge[k]/std::sqrt(dx*dx+dy*dy);
}

Universe::Float EvaluatePotential(float x, float y);
extern Universe::Float ChargeScale;

// Draw row of pixels using given corresponding potential values in p
inline void DrawPotentialRow(NimblePixel* out, float p[], size_t n) {
    using namespace Universe;
    Float lowerLimit = 0;
    Float upperLimit = CLUT_SIZE-1;
    for(int j=0; j<n; ++j) {
        float v = p[j]*ChargeScale + CLUT_SIZE/2 + 0.5f;
        if(v<=upperLimit); else v=upperLimit;
        if(v>=lowerLimit); else v=lowerLimit;
        out[j] = Clut[int(v)];
    }
}

#endif