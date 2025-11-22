#pragma once

#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include "memory"
#include "iostream"


void debug1(){
    
    char out[12];
    std::snprintf(out, sizeof(out),"%f",(float)GetFrameTime());
    DrawText(out,10,25,20,LIGHTGRAY);
};
