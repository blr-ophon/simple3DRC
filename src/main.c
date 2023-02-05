#include "main.h"
#include <stdbool.h>
#include <assert.h>
#include <math.h>

//TODO: Problem with casted rays at y < 32 positions, when looking down
//TODO: use first person view for controls. Create a vector speed with same direction
//as dir, this vector applies movement to Player position while 'w' or 's' is pressed. An
//orthogonal vector works the same for 'a' and 'd'
//TODO: Multiple modes by pressig F1, F2, F3:
//One mode is the game with textures, the other is game with no textures and the third
//is game with 2d view.

bool running = true;
bool DebugMode = false;

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
    1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,1,
    1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0
};

GameObject PlayerObj = {
    {64,64},    //pos
    {0,0},      //speed
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
    int CollumX = GAME_X + (WINDOW_WIDTH - GAME_X)/2;
    int temp = CollumX;
    castRayToCollision(renderer, VectorDir);

    float rayDir[] = {VectorDir[0], VectorDir[1]};
    bool reverseOrientation = 0;
    for(int j = 0; j < 2; j ++){
        for(int i = 0; i < (RAY_NUMBER-1)/2; i++){ //generate 30 rays to one side (right)
            RayObj castedRay = castRayToCollision(renderer, rayDir);
            float RayDist = castedRay.size;

            //Calculate distance of Collision Point Pc to camera plane c: d(Pc-c) = size*cos(a)
            //cos(a) = (v*i)/(|v|*|i|)
            float ProjectionCos = rayDir[0]*VectorDir[0] + rayDir[1]*VectorDir[1];
            RayDist *= ProjectionCos;

            //H/h = d/MapS, supposing eye can only see an entire wall at a MapS distance.
            //Anything closer to eye is out of view and so lineH = GAME_HEIGHT
            int lineH = round((mapS*GAME_HEIGHT)/RayDist); if(lineH > GAME_HEIGHT) {lineH = GAME_HEIGHT;}
            //This is the offset to put the center of the drawed line in the center of the game screen, and
            //then add the screen Y position 
            int lineO = (GAME_HEIGHT/2 - lineH/2) + GAME_Y;

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
        CollumX = temp - CAST_3D_OFFSET;
        reverseOrientation = 1;
    }
}

RayObj castRayToCollision(SDL_Renderer *renderer, float VectorDir[2]){
    float CollisionPoint[2];
    float RayVecLines[2] = {PlayerObj.pos[0], PlayerObj.pos[1]};
    float RayVecCollums[2] = {PlayerObj.pos[0], PlayerObj.pos[1]};

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
    castedRay.size = sizeVL < sizeVC? sizeVL : sizeVC;
    castedRay.horizontal = sizeVL < sizeVC? 0 : 1;
    
    //TODO: Make this optional for a debug mode
    SDL_SetRenderDrawColor(renderer, 55, 0, 55, 255);
    if(castedRay.horizontal) SDL_SetRenderDrawColor(renderer, 155, 0, 155, 255);
    SDL_Rect ColPoint = { CollisionPoint[0], CollisionPoint[1], 4, 4};
    SDL_RenderFillRect(renderer, &ColPoint);
    SDL_SetRenderDrawColor(renderer, 0, 0, 155, 255);
    SDL_RenderDrawLine(renderer, PlayerObj.pos[0], PlayerObj.pos[1], CollisionPoint[0], CollisionPoint[1]);
    return castedRay;
}


//castRayNext functions: Gets a position P(x,y), which is expected to be the point of previous
//line/collum encounter, and updates it to the next line/collum, returning the offset size
float castRayNextCollum(float VectorDir[2], float PointP[2]){
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

float castRayNextLine(float VectorDir[2], float PointP[2]){
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
float castRayFirstLine(float VectorDir[2], float PointP[2]){
    //Uses (VectorDirX)/(distance to line) ratio, multiply VectorDir by that ratio and
    //adds to the initial point to find the desired position 
    float OffsetVec[] = {VectorDir[0],VectorDir[1]};
    float P1Ratio = mapS;
    //When one of the vectors is ortogonal, it must be always the largest size possible
    //or simply larger than the max size of the other vector, which is 64/|d|. That's why
    //P1Ratio has this value when VectorDir[1], aka Yd, is 0.
    if(VectorDir[1] != 0){
        float delta_Y1 = VectorDir[1] > 0? mapS - fmod(PlayerObj.pos[1],mapS) : fmod(PlayerObj.pos[1],mapS);
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

float castRayFirstCollum(float VectorDir[2], float PointP[2]){
    float OffsetVec[] = {VectorDir[0],VectorDir[1]};
    float P1Ratio = mapS;
    if(VectorDir[0] != 0){
        float delta_X1 = VectorDir[0] > 0? mapS - fmod(PlayerObj.pos[0],mapS) : fmod(PlayerObj.pos[0],mapS);
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
        PlayerObj.pos[0]-PlayerObj.size/2,
        PlayerObj.pos[1]-PlayerObj.size/2,
        PlayerObj.size,
        PlayerObj.size
    };
    SDL_RenderFillRect(display->renderer, &playerRect);

    //Vector d
    float VectorDir[] = {cos(PlayerObj.angle), sin(PlayerObj.angle)};
    float point_E[] = {PlayerObj.pos[0] + DIR_VEC_SIZE*VectorDir[0], PlayerObj.pos[1] + DIR_VEC_SIZE*VectorDir[1]};
    SDL_SetRenderDrawColor(display->renderer, 0, 155, 0, 255);
    SDL_RenderDrawLine(display->renderer, PlayerObj.pos[0], PlayerObj.pos[1], point_E[0], point_E[1]);

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

    SDL_SetRenderDrawColor(display->renderer, 155, 155, 0, 255);
    SDL_RenderDrawLine(display->renderer, PlayerObj.pos[0], PlayerObj.pos[1],
            PlayerObj.pos[0] + 10*PlayerObj.speed[0], PlayerObj.pos[1] + 10*PlayerObj.speed[1]);
    //TODO: Implement movement as delta time function
    //(x,y) movement
    if(!IsColliding(PlayerObj.pos[0] + 2*PlayerObj.speed[0], PlayerObj.pos[1] + 2*PlayerObj.speed[1])){
        //TODO: Fix this collision based on speed
        float newX = PlayerObj.pos[0] + PlayerObj.speed[0];
        if(newX > 0 && newX < 512) PlayerObj.pos[0] = (newX);
        float newY = PlayerObj.pos[1] + PlayerObj.speed[1];
        if(newY > 0 && newY < 512) PlayerObj.pos[1] = (newY);
    }
    //view direction movement
    display->last_frame_t = SDL_GetTicks();
}

bool keymap[4];

void process_input(void){
    SDL_Event event;
    SDL_PollEvent(&event);
    switch(event.type){
        case SDL_QUIT:
            break;
		case SDL_KEYDOWN:
			if(event.key.keysym.sym == SDLK_ESCAPE) {running = false;}
			if(event.key.keysym.sym == SDLK_d) {keymap[KEY_TILT_RIGHT] = 1;}
			if(event.key.keysym.sym == SDLK_a) {keymap[KEY_TILT_LEFT] = 1;}
			if(event.key.keysym.sym == SDLK_w) {keymap[KEY_FORWARD] = 1;}
			if(event.key.keysym.sym == SDLK_s) {keymap[KEY_BACKWARD] = 1;}
            break;
		case SDL_KEYUP:
			if(event.key.keysym.sym == SDLK_d) {keymap[KEY_TILT_RIGHT] = 0;}
			if(event.key.keysym.sym == SDLK_a) {keymap[KEY_TILT_LEFT] = 0;}
			if(event.key.keysym.sym == SDLK_w) {keymap[KEY_FORWARD] = 0;}
			if(event.key.keysym.sym == SDLK_s) {keymap[KEY_BACKWARD] = 0;}
            break;
        default:
            break;
    }

    //TODO: see why sometimes speed vector and direction vector are different.
    //Draw speed vector and see whats happening, like its size and direction varying
    //It has to do with int and float calculations, 
    if(keymap[KEY_TILT_LEFT]) {PlayerObj.turn_speed = -TURN_SPEED;}
    if(keymap[KEY_TILT_RIGHT]) {PlayerObj.turn_speed = TURN_SPEED;}
    if(!keymap[KEY_TILT_LEFT] && !keymap[KEY_TILT_RIGHT]) {PlayerObj.turn_speed = 0;}
    PlayerObj.angle += PlayerObj.turn_speed; 

    if(keymap[KEY_FORWARD]){
        PlayerObj.speed[0] = PLAYER_SPEED*cos(PlayerObj.angle);
        PlayerObj.speed[1] = PLAYER_SPEED*sin(PlayerObj.angle);
    }
    if(keymap[KEY_BACKWARD]){
        PlayerObj.speed[0] = 0 - PLAYER_SPEED*cos(PlayerObj.angle);
        PlayerObj.speed[1] = 0 - PLAYER_SPEED*sin(PlayerObj.angle);
    }
    if(!keymap[KEY_FORWARD] && !keymap[KEY_BACKWARD]) {
        PlayerObj.speed[0] = 0;
        PlayerObj.speed[1] = 0;
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

