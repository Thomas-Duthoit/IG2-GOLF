#include <stdio.h>
#include <raylib/raylib.h>

int main() {
    printf("Hello, MCS !\n");
    
    InitWindow(800, 450, "Hello, MCS !");

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Hello, MCS !", 190, 200, 20, LIGHTGRAY);

            DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}