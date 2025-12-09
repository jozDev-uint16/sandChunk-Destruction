#pragma once

#include "raylib.h"
#include "raymath.h"
#include "math.h"

#include "particleEngn.hpp"

#include "memory"
#include "vector"
#include "algorithm"

struct brush{
    Vector2         lastPos, nowPos;
    particleBox&    box;
    enum state : uint8_t{
        IDLE, PLOT, ADD
    }   state;

    brush(particleBox& b): 
        lastPos({0,0}),
        nowPos({0,0}),
        box(b),
        state(IDLE) {};


    void drawBrush(Vector2 cursor, MouseButton mouse, bool keydown, elementID element){
        Vector2 boxLastPos  = Vector2Scale      (this->lastPos,1/(float)this->box.scalePtcs);
        Vector2 boxNowPos   = Vector2Scale      (this->nowPos,1/(float)this->box.scalePtcs);
        Vector2 deltaPos    = Vector2Subtract   (this->nowPos, this->lastPos);
        char*   mode;

        this->nowPos    = cursor;

        switch (this->state){
        case IDLE:{
            mode = "IDLE";
            this->lastPos   = cursor;
            

            if (!keydown) break;

            this->state     = PLOT;
            break;
        }
        case PLOT:{
            mode = "PLOT";
            DrawRectangleLines(
            this->lastPos.x,this->lastPos.y,
            deltaPos.x,deltaPos.y, RAYWHITE);

            if (keydown) break;

            this->state     = ADD;
            break;
        }
        case ADD:{
            mode = "ADD";
            int wide    = (int)(deltaPos.x/box.scalePtcs); 
            int max     = (int)((deltaPos.x * deltaPos.y)/box.scalePtcs);

            for (int p = 0; p < max; p++){
                box.particleAdd(
                    boxLastPos.x + (p % wide),
                    boxLastPos.y + (p / wide),
                    element
                );
            }
            
            this->state     = IDLE;
            break;
        }
        default:{
            this->state     = IDLE;
            break;
        }
        }
        
        DrawText(mode,nowPos.x,nowPos.y,5,GRAY);
    }
};