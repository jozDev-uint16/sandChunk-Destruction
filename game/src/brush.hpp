#pragma once

#include "raylib.h"
#include "raymath.h"

#include "particle.hpp"

#include <math.h>
#include "memory"
#include "algorithm"


struct painter{
    Vector2 pos;
    uint8_t typeIdx;      // scrolls and basically
    ptcType typeCurrent;       // can be scrolled 
    bool isDrawing;

    ptcBox& box;
    int radius;
    float density;

    painter(Vector2 offset, ptcBox& box, int rad, float d):    
        pos((Vector2){0,0}), typeIdx(0), typeCurrent(ptcType::EMPTY), isDrawing(false), box(box), radius(rad), density (d)  
        {};

    void updatePaint(MouseButton key);

    void activatePaint(int centerX, int centerY, int rad, ptcType addElement, float density);
    void drawBrush();

};


