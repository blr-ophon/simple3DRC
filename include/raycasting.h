#ifndef RAYCASTING_H
#define RAYCASTING_H
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "config.h"
#include "textures.h"

typedef struct ray_object{
    float distance;
    bool horizontal;
    float endP[2];
    int colorIndex;
    int textureXindex;
}RayObj;

struct MapObj;

bool IsColliding(int x, int y, struct MapObj *map);

void RotateVecUnit(float Vector[2], float UnitAngleCos, float UnitAngleSin, bool reverse);

void castRays(SDL_Renderer *renderer, float PlayerPos[], float VectorDir[], struct MapObj *map, TextureMap *texture);

RayObj *castRayToCollision(float PlayerPos[], float VectorDir[], struct MapObj *map, TextureMap *texture);

float castRayNextCollum(float *VectorDir, float *PointP, float mapS);

float castRayNextLine(float *VectorDir, float *PointP, float mapS);

float castRayFirstCollum(float *VectorDir, float *PointP, float mapS);

float castRayFirstLine(float *VectorDir, float *PointP, float mapS);

#endif
