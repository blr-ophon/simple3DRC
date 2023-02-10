#include "raycasting.h"
#include "main.h"

bool IsColliding(int x, int y, struct MapObj *map){
    //TODO: this doesnt work properly for x and y = 0; Flickering was removed, but
    //now dist division makes vertical bar appear on grid divisions
    //TODO x/mapS and y/mapS as constants
    if(x <= 0 || x/map->mapS >= map->mapX  || y <= 0 || y/map->mapS >= map->mapY) return true;
    return map->mapgrid[map->mapX*(y/map->mapS) + x/map->mapS];
}

void RotateVecUnit(float Vector[2], float UnitAngleCos, float UnitAngleSin, bool reverse){
    //Rotate vector by an unit angle
    float temp[] = {Vector[0], Vector[1]};
    float orientation = reverse? -1 : 1;
    Vector[0] = UnitAngleCos*temp[0] - orientation*UnitAngleSin*temp[1];
    Vector[1] = orientation*UnitAngleSin*temp[0] + UnitAngleCos*temp[1];
}


void castRays(SDL_Renderer *renderer, float PlayerPos[], float VectorDir[], struct MapObj *map, TextureMap *texture){
    float UnitAngleSin = sin(UNIT_ANGLE);
    float UnitAngleCos = cos(UNIT_ANGLE);
    int CollumX = GAME_X + GAME_WIDTH/2;
    int temp = CollumX;

    float rayDir[] = {VectorDir[0], VectorDir[1]};
    bool reverseOrientation = 0;
    for(int j = 0; j < 2; j ++){ //casts half of the rays, once to the right and then to the left
        for(int i = 0; i < (RAY_NUMBER-1)/2; i++){ //generate 30 rays to one side (right)
            RayObj *castedRay = castRayToCollision(PlayerPos, rayDir, map, texture);
            float RayDist = castedRay->distance;

            //Calculate distance of Collision Point Pc to camera plane c: d(Pc-c) = size*cos(a)
            //cos(a) = (v*i)/(|v|*|i|)
            float ProjectionCos = rayDir[0]*VectorDir[0] + rayDir[1]*VectorDir[1];
            RayDist *= ProjectionCos;

            //H/h = d/MapS, supposing eye can only see an entire wall at a MapS distance.
            //Anything closer to eye is out of view and so lineH = GAME_HEIGHT
            float lineH = round((map->mapS*GAME_HEIGHT)/RayDist); 
            //This is the offset to put the center of the drawed line in the center of the game screen, and
            //then add the screen Y position 
            float lineY = (GAME_HEIGHT/2 - lineH/2) + GAME_Y;

            DrawTextures(renderer, castedRay, lineH, lineY, CollumX, texture);

            //2d ray on minimap
            SDL_SetRenderDrawColor(renderer, 0, 0, 155, 255);
            SDL_RenderDrawLine(renderer, 
                    MAP_SCALING*PlayerPos[0], MAP_SCALING*PlayerPos[1],
                    MAP_SCALING*castedRay->endP[0], MAP_SCALING*castedRay->endP[1]);

            //Reposition CollumX and rotate vector to draw another line
            CollumX = reverseOrientation? CollumX - CAST_3D_OFFSET : CollumX + CAST_3D_OFFSET;
            RotateVecUnit(rayDir, UnitAngleCos, UnitAngleSin, reverseOrientation);

            free(castedRay);
        }
        //return to normal position and do the same for inverse direction
        rayDir[0] = VectorDir[0];
        rayDir[1] = VectorDir[1];
        RotateVecUnit(rayDir, UnitAngleCos, UnitAngleSin, 1);
        CollumX = temp - CAST_3D_OFFSET;
        reverseOrientation = 1;
    }
}

RayObj *castRayToCollision(float PlayerPos[], float VectorDir[], struct MapObj *map, TextureMap *texture){
    RayObj *castedRay = malloc(sizeof(RayObj)); //set direction for simple lighting effect
    float RayVecLines[2] = {PlayerPos[0], PlayerPos[1]};
    float RayVecCollums[2] = {PlayerPos[0], PlayerPos[1]};

    //Offset for Collision detection
    float OffsetVec[2];
    OffsetVec[0] = VectorDir[0] > 0? 0 : -1;
    OffsetVec[1] = VectorDir[1] > 0? 0 : -1;

    float sizeVL = 0;
    float sizeVC = 0;
    bool firstGridLine = 1;
    bool firstGridCollum = 1;
    while(true){
        if(sizeVL < sizeVC){ //Increase smallest value
            if(firstGridLine){ //first collision is calculated differently in other function
                sizeVL += castRayFirstLine(VectorDir, RayVecLines, map->mapS);
                firstGridLine = 0;
            }else sizeVL += castRayNextLine(VectorDir, RayVecLines, map->mapS);
        }else{
            if(firstGridCollum){
                sizeVC += castRayFirstCollum(VectorDir, RayVecCollums, map->mapS);
                firstGridCollum = 0;
            }else sizeVC += castRayNextCollum(VectorDir, RayVecCollums, map->mapS);
        }

        if(sizeVL < sizeVC){ //End Point is the smallest after the increase
            memcpy(castedRay->endP, RayVecLines, 2*sizeof(float));
            if(IsColliding(castedRay->endP[0], castedRay->endP[1] + OffsetVec[1], map)) break;
        }else {
            memcpy(castedRay->endP, RayVecCollums, 2*sizeof(float));
            if(IsColliding(castedRay->endP[0]+OffsetVec[0], castedRay->endP[1], map)) break;
        }
    }

    castedRay->distance = sizeVL < sizeVC? sizeVL : sizeVC;
    castedRay->horizontal = sizeVL < sizeVC? 0 : 1;

    //TODO: This fixes the wall color problem, but makes walls in index 1 have the same bug
    //as the ones in index 0 had, this requires a fix in collision check
    getRayTexture(castedRay, VectorDir, map->mapgrid, map->mapX, map->mapS, texture);
    
    return castedRay;
}


//castRayNext functions: Gets a position P(x,y), which is expected to be the point of previous
//line/collum encounter, and updates it to the next line/collum, returning the offset size
float castRayNextCollum(float *VectorDir, float *PointP, float mapS){
    if(VectorDir[0] == 0) {return 0;}   //Case of vector parallel to Oy
    float nextP[2];
    int orientation = VectorDir[0] > 0? 1 : -1;
    nextP[0] = PointP[0] + mapS*orientation; 
    nextP[1] = PointP[1] + mapS*(VectorDir[1]/VectorDir[0])*orientation; //mapS * vector tangent
    //Vector size by Pythagoras
    float size = sqrt(pow(nextP[0] - PointP[0], 2)+pow(nextP[1] - PointP[1], 2));                                           
    memcpy(PointP, nextP, sizeof(nextP));
    return size;
}

float castRayNextLine(float *VectorDir, float *PointP, float mapS){
    if(VectorDir[1] == 0) {return 0;}   //Case of vector parallel to Ox
    float nextP[2];
    int orientation = VectorDir[1] > 0? 1 : -1; 
    nextP[1] = PointP[1] + mapS*orientation; 
    nextP[0] = PointP[0] + mapS*(VectorDir[0]/VectorDir[1])*orientation; //mapS * 1/(vector tangent)
    //Vector size by Pythagoras
    float size = sqrt(pow(nextP[0] - PointP[0], 2)+pow(nextP[1] - PointP[1], 2));                                           
    memcpy(PointP, nextP, sizeof(nextP));
    return size;
}

//castRayFirst functions: Using a position P(x,y), which is expected to be a copy
//of Player position, then cast this point to the closest line/collum dir vector encounters.
//Returns the offset size, which is also the total size in this case.
float castRayFirstLine(float *VectorDir, float *PointP, float mapS){
    //Uses (VectorDirX)/(distance to line) ratio, multiply VectorDir by that ratio and
    //adds to the initial point to find the desired position 
    float OffsetVec[] = {VectorDir[0],VectorDir[1]};
    float P1Ratio = mapS;
    //When one of the vectors is ortogonal, it must be always the largest size possible
    //or simply larger than the max size of the other vector, which is 64/|d|. That's why
    //P1Ratio has this value when VectorDir[1], aka Yd, is 0.
    if(VectorDir[1] != 0){
        float delta_Y1 = VectorDir[1] > 0? mapS - fmod(PointP[1],mapS) : fmod(PointP[1],mapS);
        P1Ratio = delta_Y1/fabs(VectorDir[1]);
    }
    OffsetVec[0] *= P1Ratio;
    OffsetVec[1] *= P1Ratio;
    PointP[0] += OffsetVec[0];
    if(PointP[0] < 0) {PointP[0] = 0;}
    PointP[1] += OffsetVec[1];
    if(PointP[1] < 0) {PointP[1] = 0;}

    float size = P1Ratio;
    return size;
}

float castRayFirstCollum(float *VectorDir, float *PointP, float mapS){
    float OffsetVec[] = {VectorDir[0],VectorDir[1]};
    float P1Ratio = mapS;
    if(VectorDir[0] != 0){
        float delta_X1 = VectorDir[0] > 0? mapS - fmod(PointP[0],mapS) : fmod(PointP[0],mapS);
        P1Ratio = delta_X1/fabs(VectorDir[0]);
    }
    OffsetVec[0] *= P1Ratio;
    OffsetVec[1] *= P1Ratio;
    PointP[0] += OffsetVec[0];
    if(PointP[0] < 0) {PointP[0] = 0;}
    PointP[1] += OffsetVec[1];
    if(PointP[1] < 0) {PointP[1] = 0;}

    float size = P1Ratio;
    return size;
}

