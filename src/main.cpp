#include <raylib.h>

#define SCREENWIDE  800
#define SCREENHIGH  800
#define TARGET_FPS  144

#include "particleEngn.hpp"

void gameInit(){
    SetConfigFlags  (FLAG_WINDOW_RESIZABLE);
    InitWindow      (SCREENWIDE, SCREENHIGH, "SandBeaker v0.0.5");
    SetTargetFPS    (TARGET_FPS);

    initialElements();
}


int main() 
{
    gameInit();

    //
    particleBox testChunk({3,18},PTC_SCALE);
    
    while (!WindowShouldClose()){
        
        testChunk.boxUpdate();
        testChunk.particleAdd(
            GetRandomValue(110,135),
            GetRandomValue(0,5),
            elementID::POWDER);

        testChunk.boxReset(IsKeyPressed(KEY_X));

        BeginDrawing();
        ClearBackground(BLANK);
        DrawFPS(0,0);



        testChunk.boxRender(IsKeyDown(KEY_TAB),(int)GetMouseWheelMove());

        EndDrawing();
    }
    
    CloseWindow();
}