#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include "memory"
#include "iostream"

#include "cell.hpp"
#include "brush.hpp"

#define TARGET_FPS  (double)(144)
#define PHY_STEP    (double)(1/TARGET_FPS)
#define WINDOW_DIMENSIONS (Vector2){900,900}

uint16_t frames = 0;
double accumTick = 0.0; 

cellDomain firstChunk(255,255,CELL_SCALE,(Vector2){10,50});
brush firstBrush({0,0},firstChunk,5,0.5f);


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
        
    accumTick += (double)(GetFrameTime());

    while (accumTick >= PHY_STEP){
    
        firstBrush.updateBrush(MOUSE_BUTTON_LEFT);

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


