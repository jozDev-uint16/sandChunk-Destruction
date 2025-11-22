#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include "memory"
#include "iostream"

#include "cell.hpp"
#include "debug.hpp"

#define TARGET_FPS  (double)(144)
#define PHY_STEP    (double)(1/TARGET_FPS)
#define WINDOW_DIMENSIONS (Vector2){900,900}

uint16_t frames = 0;
double accumTick = 0.0; 

cellDomain firstChunk(255,255,CELL_SCALE,(Vector2){10,50});

void drawBrush(cellDomain& chunk, int centerX, int centerY, int radius, cName material, float density) {
    
    // 1. Iterate over a bounding box around the mouse
    // We use 'int' to allow negative values (off-screen) without crashing
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {

            // 2. Circular Mask Check (Pythagoras: x*x + y*y <= r*r)
            // If outside the circle, skip.
            if ((x * x) + (y * y) > (radius * radius)) continue;

            // 3. "Spray Paint" Randomness Check
            // If density is 1.0, it fills completely. If 0.5, it fills 50% of the pixels.
            // GetRandomValue returns 0-100, so we divide by 100.0f
            if ((GetRandomValue(0, 100) / 100.0f) > density) continue;

            // 4. Calculate Target Coordinates
            int targetX = centerX + x;
            int targetY = centerY + y;

            // 5. Call your existing Add function
            // Note: cellAdd/addParticle should handle bounds checking internally!
            chunk.cellAdd((uint8_t)targetX, (uint8_t)targetY, material);
        }
    }
};

void GameInit()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_DIMENSIONS.x, WINDOW_DIMENSIONS.y, "SandEngine");
    SetTargetFPS(TARGET_FPS);

    // load resources (DO NOT SET)

    
    initElementRecipe();
    initCellPalette();      // just added this here so that it auto-works without doing it globally
}

void GameCleanup()
{
    // unload resources

    CloseWindow();
}

bool GameUpdate()
{
    
    // mouse funct
    Vector2 mouseTranslate = (Vector2){(GetMouseX()-firstChunk.boxPos.x)/firstChunk.cellSize,(GetMouseY()-firstChunk.boxPos.y)/firstChunk.cellSize};

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) drawBrush(firstChunk, mouseTranslate.x, mouseTranslate.y, 5,  NONE, 0.9f);
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) drawBrush(firstChunk, mouseTranslate.x, mouseTranslate.y, 3,  WOOD, 0.5f);
        
    accumTick += (double)(GetFrameTime());

    while (accumTick >= PHY_STEP){
        drawBrush(firstChunk, 185, 10, 3,  GRAVEL, 0.5f);
        drawBrush(firstChunk, 85, 10, 3,  SAND, 0.5f);

        firstChunk.domainUpdate(frames++);
        accumTick -= PHY_STEP;
    }

    if (IsKeyPressed(KEY_X)) firstChunk.domainReset();
    return true;
}

void GameDraw()
{
    BeginDrawing();
    ClearBackground(BLACK);
    
    debug1();
    
    DrawRectangleLines(
        firstChunk.boxPos.x-1,
        firstChunk.boxPos.y-1,
        2+firstChunk.xBounds*firstChunk.cellSize,
        2+firstChunk.yBounds*firstChunk.cellSize,
        RED
    );
    firstChunk.domainRender();

    DrawText("Sand!", 10, 10, 20, RAYWHITE);
    DrawFPS(15,GetScreenHeight() - 20);

    EndDrawing();
}

int main()
{
    GameInit();

    while (!WindowShouldClose())
    {
        if (!GameUpdate()) break;

        
        GameDraw();
    }
    GameCleanup();

    return 0;
};


