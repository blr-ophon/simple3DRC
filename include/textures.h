#ifndef TEXTURES_H
#define TEXTURES_H
#include <SDL2/SDL.h>

struct ray_object;

void getRayTexture(struct ray_object *castedRay, float *VectorDir, int *mapgrid, int mapX, int mapS);

void DrawTextures(SDL_Renderer *renderer, struct ray_object *castedRay, float lineH, float lineY, int CollumX);

SDL_Color IndexToColor(int Index);

void shadeColor(SDL_Color *color);

#endif
