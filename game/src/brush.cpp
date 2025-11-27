#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include "memory"
#include "algorithm"
#include "iostream"

#include "brush.hpp"

// REVISED

void painter::updatePaint(MouseButton key){
    this->isDrawing = false;
    this->pos = (Vector2){  (float)(GetMouseX()), (float)(GetMouseY()) };

    int cIndex = (int)(this->typeIdx);
    int delta = (int)(GetMouseWheelMove());

    const char* label = elementRegistry[(uint8_t)typeCurrent].name;
    if(label == nullptr) label = "Eraser!";

    DrawText(label,pos.x,pos.y+15,
        (this->box.ptcScale*5),elementRegistry[(uint8_t)typeCurrent].color);

    if (delta != 0){
        cIndex += delta;   

        if (cIndex < 0) {
            cIndex = (uint8_t)ptcType::_MAX_TYPE_COUNT - 1; // Wrap to end
        } else if (cIndex >= (uint8_t)ptcType::_MAX_TYPE_COUNT) {
            cIndex = 0;             // Wrap to start
        };

        // 4. CRITICAL: Save the new index back to the CLASS MEMBER
        this->typeIdx = (uint8_t)cIndex;
        this->typeCurrent = (ptcType)cIndex;
    };

    

    if(!IsMouseButtonDown(key)) return;

    this->isDrawing = true;
    activatePaint(
        this->pos.x,
        this->pos.y,
        this->radius,
        this->typeCurrent,
        this->density);
};

void painter::activatePaint(int centerX, int centerY, int rad, ptcType addElement, float density){
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
            int targetX = ((centerX-this->box.xyBox.x)/box.ptcScale) + x;
            int targetY = ((centerY-this->box.xyBox.y)/box.ptcScale) + y;

            // 5. Call your existing Add function
            // Note: cellAdd/addParticle should handle bounds checking internally!
            this->box.boxAdd((uint8_t)targetX, (uint8_t)targetY, addElement);
        }
    }
};