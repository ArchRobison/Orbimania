#pragma once
#ifndef View_H
#define View_H

extern int ZoomLevel;
void SetZoom(int level, int centerX, int centerY);
void RecenterView(int centerX, int centerY);

// Pixel at [i][j] mapps to universe coordinate (ViewScale*j + ViewOffsetX, ViewScale*i + ViewOffsetY)
extern float ViewScale;
extern float ViewOffsetX, ViewOffsetY;
extern float ViewVelocityScale;
extern float ViewMassScale;
extern float ViewChargeScale;

#endif /* View_H */

