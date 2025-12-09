#include <raylib.h>

#define SCREENWIDE  800
#define SCREENHIGH  800
#define TARGET_FPS  144

#include "particleEngn.hpp"
#include "brushObj.hpp"

void gameInit(){
    SetConfigFlags  (FLAG_WINDOW_RESIZABLE);
    InitWindow      (SCREENWIDE, SCREENHIGH, "SandBeaker v0.0.5");
    SetTargetFPS    (TARGET_FPS);

    initialElements();
};


int main() 
{
    gameInit();

    //
    particleBox testChunk({3,18},PTC_SCALE);    
    brush       testBrush(testChunk);    
    
    while (!WindowShouldClose()){

        testBrush.drawBrush(
            GetMousePosition(),
            MOUSE_BUTTON_LEFT,
            IsMouseButtonDown(MOUSE_BUTTON_LEFT),
            elementID::SAND
        );


        testChunk.boxUpdate();
        testChunk.boxReset(IsKeyPressed(KEY_X));

        BeginDrawing();
        ClearBackground(BLANK);
        DrawFPS(0,0);



        testChunk.boxRender(IsKeyDown(KEY_TAB),(int)GetMouseWheelMove());

        EndDrawing();
    }
    
    CloseWindow();
};

