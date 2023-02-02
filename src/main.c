#include "main.h"
#include <stdbool.h>
#include <assert.h>
#include <math.h>

//TODO: Problem with casted rays at y < 32 positions, when looking down
//TODO: use first person view for controls. Create a vector speed with same direction
//as dir, this vector applies movement to Player position while 'w' or 's' is pressed. An
//orthogonal vector works the same for 'a' and 'd'

bool running = true;

//Precalculated values
float UnitAngleSin; 
float UnitAngleCos;

int mapX = 16, mapY = 16, mapS = 32;
int mapgrid[] = {
    1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,1,
    0,0,0,0,1,0,0,0,0,1,0,1,1,1,0,1,
    1,1,0,0,1,1,1,1,0,1,0,1,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,1,
    1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,
    1,0,1,0,0,1,1,1,0,1,1,1,0,1,0,1,
    1,1,1,0,1,1,1,1,0,0,0,0,0,0,0,1,
    1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,1,
    1,0,0,0,1,1,1,0,0,0,1,1,0,0,0,1,
    1,0,1,0,1,0,0,0,0,0,1,1,0,0,0,1,
    1,0,0,0,1,0,0,0,0,1,1,1,1,0,0,1,
    1,0,0,0,1,0,1,0,1,1,1,1,1,1,0,1,
    1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,1,
    1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0
};

GameObject PlayerObj = {
    64,    //x
    64,    //y
    0,      //x_speed
    0,      //y_speed
    8,      //size
    2,      //angle
    0       //turn_speed
};

bool IsColliding(int x, int y){
    //TODO x/mapS and y/mapS as constants
    if(x < 0 || x/mapS >= mapX  || y < 0 || y/mapS >= mapY) return true;
    return mapgrid[mapX*(y/mapS) + x/mapS];
}

void RotateVecUnit(float Vector[2], bool reverse){
    //Rotate vector by an unit angle
    float temp[] = {Vector[0], Vector[1]};
    float orientation = reverse? -1 : 1;
    Vector[0] = UnitAngleCos*temp[0] - orientation*UnitAngleSin*temp[1];
    Vector[1] = orientation*UnitAngleSin*temp[0] + UnitAngleCos*temp[1];
}

void castRays(SDL_Renderer *renderer, float VectorDir[2] ){
    int CollumX = 512 + 256;
    castRayToCollision(renderer, VectorDir);

    float rayDir[] = {VectorDir[0], VectorDir[1]};
    bool reverseOrientation = 0;
    for(int j = 0; j < 2; j ++){
        for(int i = 0; i < (RAY_NUMBER-1)/2; i++){ //generate 30 rays to one side (right)
            RayObj castedRay = castRayToCollision(renderer, rayDir);
            float RayDist = castedRay.size;

            //cos(a) = (v*i)/(|v|*|i|)
            float ProjectionCos = rayDir[0]*VectorDir[0] + rayDir[1]*VectorDir[1];
            RayDist *= ProjectionCos;

            float lineH = (mapS*320)/RayDist; 
            float lineO = 160-lineH/2;

            SDL_SetRenderDrawColor(renderer, 55, 0, 55, 255);
            if(castedRay.horizontal) SDL_SetRenderDrawColor(renderer, 155, 0, 155, 255);
            SDL_Rect GameCollumRender = {CollumX, lineO, CAST_3D_OFFSET, lineH};
            CollumX = reverseOrientation? CollumX - CAST_3D_OFFSET : CollumX + CAST_3D_OFFSET;
            SDL_RenderFillRect(renderer, &GameCollumRender);
            RotateVecUnit(rayDir, reverseOrientation);
        }
        //return to normal position and do the same for inverse direction
        rayDir[0] = VectorDir[0];
        rayDir[1] = VectorDir[1];
        RotateVecUnit(rayDir, 1);
        CollumX = 512 + 256 - CAST_3D_OFFSET;
        reverseOrientation = 1;
    }
}

RayObj castRayToCollision(SDL_Renderer *renderer, float VectorDir[2]){
    float CollisionPoint[2];
    float RayVecLines[2] = {PlayerObj.x, PlayerObj.y};
    float RayVecCollums[2] = {PlayerObj.x, PlayerObj.y};

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
                sizeVL += castRayFirstLine(VectorDir, RayVecLines);
                firstGridLine = 0;
            }else sizeVL += castRayNextLine(VectorDir, RayVecLines);
        }else{
            if(firstGridCollum){
                sizeVC += castRayFirstCollum(VectorDir, RayVecCollums);
                firstGridCollum = 0;
            }else sizeVC += castRayNextCollum(VectorDir, RayVecCollums);
        }

        if(sizeVL < sizeVC){ //CollisionPoint is the smallest after the increase
            memcpy(CollisionPoint, RayVecLines, sizeof(CollisionPoint));
            if(IsColliding(CollisionPoint[0], CollisionPoint[1] + OffsetVec[1])) break;
        }else {
            memcpy(CollisionPoint, RayVecCollums, sizeof(CollisionPoint));
            if(IsColliding(CollisionPoint[0]+OffsetVec[0], CollisionPoint[1])) break;
        }
    }

    RayObj castedRay; //set direction for simple lighting effect
    if(sizeVL < sizeVC){
        castedRay.size = sizeVL;
        castedRay.horizontal = 0;
    }else{
        castedRay.size = sizeVC;
        castedRay.horizontal = 1;
    }
    
    //TODO: Make this optional for a debug mode
    SDL_SetRenderDrawColor(renderer, 55, 0, 55, 255);
    if(castedRay.horizontal) SDL_SetRenderDrawColor(renderer, 155, 0, 155, 255);
    SDL_Rect ColPoint = { CollisionPoint[0], CollisionPoint[1], 4, 4};
    SDL_RenderFillRect(renderer, &ColPoint);
    SDL_SetRenderDrawColor(renderer, 0, 0, 155, 255);
    SDL_RenderDrawLine(renderer, PlayerObj.x, PlayerObj.y, CollisionPoint[0], CollisionPoint[1]);
    return castedRay;
}


//castRayNext functions: Gets a position P(x,y), which is expected to be the point of previous
//line/collum encounter, and updates it to the next line/collum, returning the offset size
float castRayNextCollum(float VectorDir[2], float PointP[2]){
    float nextP[2];
    //TODO: test case when VectorDir[0] == 0
    int orientation = VectorDir[0] > 0? 1 : -1;
    nextP[0] = PointP[0] + mapS*orientation; 
    nextP[1] = PointP[1] + mapS*(VectorDir[1]/VectorDir[0])*orientation; //mapS * vector tangent
    //Vector size by Pythagoras
    float size = sqrt(pow(nextP[0] - PointP[0], 2)+pow(nextP[1] - PointP[1], 2));                                           
    memcpy(PointP, nextP, sizeof(nextP));
    return size;
}

float castRayNextLine(float VectorDir[2], float PointP[2]){
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
float castRayFirstLine(float VectorDir[2], float PointP[2]){
    float OffsetVec[] = {VectorDir[0],VectorDir[1]};
    float P1Ratio = mapS;
    //When one of the vectors is ortogonal, it must be always the largest size possible
    //or simply larger than the max size of the other vector, which is 64/|d|. That's why
    //P1Ratio has this value when VectorDir[1], aka Yd, is 0.
    if(VectorDir[1] != 0){
        int delta_Y1 = VectorDir[1] > 0? mapS - (((int)PlayerObj.y)%mapS) : ((int)PlayerObj.y)%mapS;
        P1Ratio = delta_Y1/fabs(VectorDir[1]);
    }
    OffsetVec[0] *= P1Ratio;
    OffsetVec[1] *= P1Ratio;
    PointP[0] += OffsetVec[0];
    PointP[1] += OffsetVec[1];

    float size = P1Ratio;
    return size;
}

float castRayFirstCollum(float VectorDir[2], float PointP[2]){
    float OffsetVec[] = {VectorDir[0],VectorDir[1]};
    float P1Ratio = mapS;
    if(VectorDir[0] != 0){
        int delta_X1 = VectorDir[0] > 0? mapS - (((int)PlayerObj.x)%mapS) : ((int)PlayerObj.x)%mapS;
        P1Ratio = delta_X1/fabs(VectorDir[0]);
    }
    OffsetVec[0] *= P1Ratio;
    OffsetVec[1] *= P1Ratio;
    PointP[0] += OffsetVec[0];
    PointP[1] += OffsetVec[1];

    float size = P1Ratio;
    return size;
}



void render_2d(DisplaySettings *display){
    SDL_SetRenderDrawColor(display->renderer, 50, 50, 50, 255);
    SDL_RenderClear(display->renderer);

    //grid
    for(int i = 0; i < mapY; i++){
        for(int j = 0; j < mapX; j++){
            SDL_Rect GameWalls = {i*mapS, j*mapS, mapS-1, mapS-1};
            SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
            if(mapgrid[mapX*j + i]){
                SDL_SetRenderDrawColor(display->renderer, 255, 255, 255, 255);
            }
            SDL_RenderFillRect(display->renderer, &GameWalls);
        }
    }

    //player
    SDL_SetRenderDrawColor(display->renderer, 255, 0, 0, 255);
    SDL_Rect playerRect = {
        PlayerObj.x-PlayerObj.size/2,
        PlayerObj.y-PlayerObj.size/2,
        PlayerObj.size,
        PlayerObj.size
    };
    SDL_RenderFillRect(display->renderer, &playerRect);

    //view direction

    //Vector d
    float VectorDir[] = {cos(PlayerObj.angle), sin(PlayerObj.angle)};
    float point_E[] = {PlayerObj.x + DIR_VEC_SIZE*VectorDir[0], PlayerObj.y + DIR_VEC_SIZE*VectorDir[1]};
    SDL_SetRenderDrawColor(display->renderer, 0, 155, 0, 255);
    SDL_RenderDrawLine(display->renderer, PlayerObj.x, PlayerObj.y, point_E[0], point_E[1]);

    //Ray and Collision points
    castRays(display->renderer, VectorDir);
    SDL_RenderPresent(display->renderer);
}

void update(DisplaySettings *display){
    int time_passed = SDL_GetTicks() - display->last_frame_t;
    int time_to_wait = FRAME_TARGET_TIME - time_passed;
    if(time_to_wait > 0){
        SDL_Delay(time_to_wait);
    }

    //TODO: Implement movement as delta time function
    //(x,y) movement
    if(!IsColliding(PlayerObj.x + PlayerObj.x_speed, PlayerObj.y + PlayerObj.y_speed)){
        float newX = PlayerObj.x + PlayerObj.x_speed;
        if(newX > 0 && newX < 1024) PlayerObj.x = newX;
        float newY = PlayerObj.y + PlayerObj.y_speed;
        if(newY > 0 && newY < 512) PlayerObj.y = newY;
    }
    //view direction movement
    PlayerObj.angle += PlayerObj.turn_speed; 
    display->last_frame_t = SDL_GetTicks();
}

void process_input(void){
    SDL_Event event;
    SDL_PollEvent(&event);
    switch(event.type){
        case SDL_QUIT:
            break;
		case SDL_KEYDOWN:
            //TODO: other switch for event.key.keysym.sym
			if(event.key.keysym.sym == SDLK_ESCAPE){
                running = false;
			}
			if(event.key.keysym.sym == SDLK_LEFT){
                PlayerObj.x_speed = -PLAYER_SPEED;
            }
			if(event.key.keysym.sym == SDLK_RIGHT){
                PlayerObj.x_speed = PLAYER_SPEED;
            }
			if(event.key.keysym.sym == SDLK_DOWN){
                PlayerObj.y_speed = PLAYER_SPEED;
            }
			if(event.key.keysym.sym == SDLK_UP){
                PlayerObj.y_speed = -PLAYER_SPEED;
            }
			if(event.key.keysym.sym == SDLK_e){
                PlayerObj.turn_speed = TURN_SPEED;
            }
			if(event.key.keysym.sym == SDLK_q){
                PlayerObj.turn_speed = -TURN_SPEED;
            }
            break;
		case SDL_KEYUP:
			if(event.key.keysym.sym == SDLK_LEFT){
                PlayerObj.x_speed = 0;
            }
			if(event.key.keysym.sym == SDLK_RIGHT){
                PlayerObj.x_speed = 0;
            }
			if(event.key.keysym.sym == SDLK_DOWN){
                PlayerObj.y_speed = 0;
            }
			if(event.key.keysym.sym == SDLK_UP){
                PlayerObj.y_speed = 0;
            }
			if(event.key.keysym.sym == SDLK_e){
                PlayerObj.turn_speed = 0;
            }
			if(event.key.keysym.sym == SDLK_q){
                PlayerObj.turn_speed = 0;
            }
            break;
        default:
            break;
    }
}

void destroy_display(DisplaySettings *display){
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
    SDL_Quit();
}

void init_display(DisplaySettings *display){
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    //display->w = DM.w;
    display->w = 1024;
    if(display->w % 2 != 0) display->w --;
    display->h = (display->w)/2;
    
    display->window = SDL_CreateWindow(
            "simple3DRC",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            display->w,
            display->h,
            SDL_WINDOW_BORDERLESS
            );
    display->renderer = SDL_CreateRenderer(display->window, -1, 0);
    display->last_frame_t = 0;
}

int main(void){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        exit(EXIT_FAILURE);
    }
    DisplaySettings display;
    init_display(&display);
    //Pre-calculating these values significantly optimizes code
    UnitAngleSin = sin(UNIT_ANGLE);
    UnitAngleCos = cos(UNIT_ANGLE);
    while(running){
        process_input();
        update(&display);
        render_2d(&display);
    }
    destroy_display(&display);
    return 0;
}

