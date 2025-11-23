#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include "memory"
#include "algorithm"
#include "iostream"

#include "brush.hpp"

void brush::updateBrush(MouseButton key){
    this->isDrawing = false;
    this->pos = (Vector2){  (float)(GetMouseX()), (float)(GetMouseY()) };

    int cIndex = (int)(this->typeIndex);
    int delta = (int)(GetMouseWheelMove());

    DrawCircle(pos.x,pos.y,
        this->radius*this->chunk.cellSize,
        (Color){100,100,100,25});

    DrawCircleLines(pos.x,pos.y,
        this->radius*this->chunk.cellSize,
        elementPalette[typeCurrent]);

    if (delta != 0){
        cIndex += delta;   

        if (cIndex < 0) {
            cIndex = MAX_TYPES - 1; // Wrap to end
        } else if (cIndex >= MAX_TYPES) {
            cIndex = 0;             // Wrap to start
        };

        // 4. CRITICAL: Save the new index back to the CLASS MEMBER
        this->typeIndex = (uint16_t)cIndex;
        this->typeCurrent = (cName)cIndex;
    };

    if(!IsMouseButtonDown(key)) return;

    this->isDrawing = true;
    activateBrush(
        this->pos.x,
        this->pos.y,
        this->radius,
        this->typeCurrent,
        this->density);
};

void brush::activateBrush(int centerX, int centerY, int rad, cName material, float density) {
    
    // 1. Iterate over a bounding box around the mouse
    // We use 'int' to allow negative values (off-screen) without crashing
    for (int y = -rad; y <= rad; y++) {
        for (int x = -rad; x <= rad; x++) {

            // 2. Circular Mask Check (Pythagoras: x*x + y*y <= r*r)
            // If outside the circle, skip.
            if ((x * x) + (y * y) > (rad * rad)) continue;

            // 3. "Spray Paint" Randomness Check
            // If density is 1.0, it fills completely. If 0.5, it fills 50% of the pixels.
            // GetRandomValue returns 0-100, so we divide by 100.0f
            if ((GetRandomValue(0, 100) / 100.0f) > density) continue;

            // 4. Calculate Target Coordinates
            int targetX = ((centerX-this->chunk.boxPos.x)/chunk.cellSize) + x;
            int targetY = ((centerY-this->chunk.boxPos.y)/chunk.cellSize) + y;

            // 5. Call your existing Add function
            // Note: cellAdd/addParticle should handle bounds checking internally!
            chunk.cellAdd((uint8_t)targetX, (uint8_t)targetY, material);
        }
    }
};