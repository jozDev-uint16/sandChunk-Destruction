#pragma once

#include "raylib.h"
#include "raymath.h"
#include "cell.hpp"

#include <math.h>
#include "memory"
#include "algorithm"

struct brush{

    Vector2 pos;
    uint8_t typeIndex;      // scrolls and basically
    cName typeCurrent;       // can be scrolled 
    bool isDrawing;

    cellDomain& chunk;
    int radius;
    float density;

    brush(Vector2 offset, cellDomain& box, int rad, float d):    
        pos((Vector2){0,0}), typeIndex(0), typeCurrent(NONE), isDrawing(false), chunk(box), radius(rad), density (d)  
        {};

    void updateBrush(MouseButton key);

    void activateBrush(int centerX, int centerY, int rad, cName material, float density);

};


