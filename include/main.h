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
    float last_frame_t;
}DisplaySettings;

typedef struct game_object{
    float pos[2];
    float speed[2];
    float size;
    float angle;
    float turn_speed;
}GameObject;

typedef struct ray_object{
    float size;
    bool horizontal;
}RayObj;

bool IsColliding(int x, int y);

void RotateVecUnit(float Vector[2], bool reverse);

void castRays(SDL_Renderer *renderer, float VectorDir[2]);

RayObj castRayToCollision(SDL_Renderer *renderer, float VectorDir[2]);

float castRayNextCollum(float VectorDir[2], float PointP[2]);

float castRayNextLine(float VectorDir[2], float PointP[2]);

float castRayFirstCollum(float VectorDir[2], float PointP[2]);

float castRayFirstLine(float VectorDir[2], float PointP[2]);

void process_input(void);

void update(DisplaySettings *display);

void init_display(DisplaySettings *display);

void destroy_display(DisplaySettings *display);

void render_2d(DisplaySettings *display);
#endif
