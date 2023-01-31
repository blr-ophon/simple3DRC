#ifndef MAIN_H
#define MAIN_H
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "config.h"

typedef struct displaySettings{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int w;
    int h;
}DisplaySettings;

typedef struct game_object{
    float x;
    float y;
    float x_speed;
    float y_speed;
    float size;
    float angle;
    float turn_speed;
}GameObject;

bool IsColliding(int x, int y);

float castRayFirstCollum(float VectorDir[2], float PointP[2]);

float castRayFirstLine(float VectorDir[2], float PointP[2]);

void process_input(void);

void init_display(DisplaySettings *display);

void destroy_display(DisplaySettings *display);

void render_2d(DisplaySettings *display);
#endif
