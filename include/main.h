#ifndef MAIN_H
#define MAIN_H
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "config.h"
#include "controls.h"
#include "textures.h"
#include "raycasting.h"

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
    float size;         //TODO: No need to have this, only purpose is debug mode. Use a constant
    float angle;
    float turn_speed;
}GameObject;

struct MapObj{
    int *mapgrid;
    int mapS;
    int mapX;
    int mapY;
};

void process_input(GameObject *PlayerObj);

void update(DisplaySettings *display, GameObject *PlayerObj, struct MapObj *map);

void init_display(DisplaySettings *display);

void destroy_display(DisplaySettings *display);

void render_2d(DisplaySettings *display, GameObject *PlayerObj, struct MapObj *map);

#endif
