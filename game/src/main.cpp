#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include "memory"
#include "iostream"

#include "particle.hpp"
#include "brush.hpp"

#define TARGET_FPS  (double)(144)
#define PHY_STEP    (double)(1/TARGET_FPS)
#define WINDOW_DIMENSIONS (Vector2){900,900}

uint16_t frames = 0;
double accumTick = 0.0; 


/*      MAIN GAME OBJECTS       */
ptcBox  originalChunk  ({10,50},3);
painter firstPaintbruh ({0,0},originalChunk,5,1.0f);

void GameInit()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_DIMENSIONS.x, WINDOW_DIMENSIONS.y, "by jozco");
    SetTargetFPS(TARGET_FPS);

    // load resources (DO NOT SET NON-GLOBALS)
    initialElements();
    initialRxnMatrix();
}


void GameCleanup()
{
    // unload resources
    originalChunk.~ptcBox();
    CloseWindow();
}

bool GameUpdate()
{
    
    // mouse funct
    firstPaintbruh.updatePaint(MOUSE_BUTTON_LEFT);
        
    accumTick += (double)(GetFrameTime());

    while (accumTick >= PHY_STEP){
        //
        originalChunk.boxUpdate(frames);


        accumTick -= PHY_STEP;
    }

    if (IsKeyPressed(KEY_X)){ 
        std::cout << "delet" << std::endl;
        originalChunk.boxClear();
    }//

    frames ++;
    return true;
}

void GameDraw()
{
    BeginDrawing();
    ClearBackground(BLACK);

    originalChunk.boxDraw(true);
    firstPaintbruh.drawBrush();

    originalChunk.boxDebug(IsKeyDown(KEY_TAB));

    DrawText("Sand Engine! v0.0.8", 10, 10, 20, RAYWHITE);
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


