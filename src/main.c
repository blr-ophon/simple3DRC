#include "main.h"
#include <stdbool.h>
#include <assert.h>
#include <math.h>

//TODO: Problem with casted rays at collum 0 and line 0 due to collision check exception
//TODO: Multiple modes by pressig F1, F2, F3:
//One mode is the game with textures, the other is game with no textures and the third
//is game with 2d view.
//TODO: Modularize code into (raycasting algorithm, vertical scanline drawing,
//color and texture handling, BMP handling, debugger mode.

bool running = true;

void render_2d(DisplaySettings *display, GameObject *PlayerObj, struct MapObj *map){
    SDL_SetRenderDrawColor(display->renderer, 20, 20, 20, 255);
    SDL_RenderClear(display->renderer);
    //paint the floor in a different collor
    SDL_SetRenderDrawColor(display->renderer, 100, 100, 100, 255);
    SDL_Rect Floor = {GAME_X, GAME_Y+GAME_HEIGHT/2, GAME_WIDTH, GAME_HEIGHT/2}; 
    SDL_RenderFillRect(display->renderer, &Floor);


    //Vector d
    //TODO: Vector d shouldnt be declared/initialized in render function
    float VectorDir[] = {cos(PlayerObj->angle), sin(PlayerObj->angle)};
    float point_E[] = {PlayerObj->pos[0] + DIR_VEC_SIZE*VectorDir[0], PlayerObj->pos[1] + DIR_VEC_SIZE*VectorDir[1]};
    SDL_SetRenderDrawColor(display->renderer, 0, 155, 0, 255);
    SDL_RenderDrawLine(display->renderer, 
            MAP_SCALING * PlayerObj->pos[0],
            MAP_SCALING * PlayerObj->pos[1],
            MAP_SCALING * point_E[0],
            MAP_SCALING * point_E[1]);

    //Ray and Collision points
    castRays(display->renderer, PlayerObj->pos, VectorDir, map);

    //grid
    for(int i = 0; i < map->mapY; i++){
        for(int j = 0; j < map->mapX; j++){
            SDL_Rect Game2DGrid = {
                MAP_SCALING * i*map->mapS, 
                MAP_SCALING * j*map->mapS,
                MAP_SCALING * (map->mapS-1),
                MAP_SCALING * (map->mapS-1)};
            SDL_SetRenderDrawColor(display->renderer, 255, 255, 255, 55);
            if(map->mapgrid[map->mapX*j + i]){
                SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
            }
            SDL_RenderFillRect(display->renderer, &Game2DGrid);
        }
    }

    //player
    SDL_SetRenderDrawColor(display->renderer, 255, 0, 0, 255);
    SDL_Rect playerRect = {
        MAP_SCALING * PlayerObj->pos[0]-PlayerObj->size/2,
        MAP_SCALING * PlayerObj->pos[1]-PlayerObj->size/2,
        PlayerObj->size,
        PlayerObj->size
    };
    SDL_RenderFillRect(display->renderer, &playerRect);

    SDL_RenderPresent(display->renderer);
}

void update(DisplaySettings *display, GameObject *PlayerObj, struct MapObj *map){
    int time_passed = SDL_GetTicks() - display->last_frame_t;
    int time_to_wait = FRAME_TARGET_TIME - time_passed;
    if(time_to_wait > 0){
        SDL_Delay(time_to_wait);
    }

    SDL_SetRenderDrawColor(display->renderer, 155, 155, 0, 255);
    SDL_RenderDrawLine(display->renderer, PlayerObj->pos[0], PlayerObj->pos[1],
            PlayerObj->pos[0] + 10*PlayerObj->speed[0], PlayerObj->pos[1] + 10*PlayerObj->speed[1]);
    //TODO: Implement movement as delta time function
    
    //(x,y) movement
        int WallDistanceX = PlayerObj->speed[0] >= 0? map->mapS/4 : -map->mapS/4;
    if(!IsColliding(PlayerObj->pos[0] + PlayerObj->speed[0] + WallDistanceX, PlayerObj->pos[1], map)){
        float newX = PlayerObj->pos[0] + PlayerObj->speed[0];
        if(newX > 0 && newX < 512) PlayerObj->pos[0] = (newX);
    }

    int WallDistanceY = PlayerObj->speed[1] >= 0? map->mapS/4 : -map->mapS/4;
    if(!IsColliding(PlayerObj->pos[0], PlayerObj->pos[1] + PlayerObj->speed[1] + WallDistanceY, map)){
        float newY = PlayerObj->pos[1] + PlayerObj->speed[1];
        if(newY > 0 && newY < 512) PlayerObj->pos[1] = (newY);
    }
    //view direction movement
    display->last_frame_t = SDL_GetTicks();
}

bool keymap[4];

void process_input(GameObject *PlayerObj){
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

    if(keymap[KEY_TILT_LEFT]) {PlayerObj->turn_speed = -TURN_SPEED;}
    if(keymap[KEY_TILT_RIGHT]) {PlayerObj->turn_speed = TURN_SPEED;}
    if(!keymap[KEY_TILT_LEFT] && !keymap[KEY_TILT_RIGHT]) {PlayerObj->turn_speed = 0;}
    PlayerObj->angle += PlayerObj->turn_speed; 

    if(keymap[KEY_FORWARD]){
        PlayerObj->speed[0] = PLAYER_SPEED*cos(PlayerObj->angle);
        PlayerObj->speed[1] = PLAYER_SPEED*sin(PlayerObj->angle);
    }
    if(keymap[KEY_BACKWARD]){
        PlayerObj->speed[0] =  -PLAYER_SPEED*cos(PlayerObj->angle);
        PlayerObj->speed[1] =  -PLAYER_SPEED*sin(PlayerObj->angle);
    }
    if(!keymap[KEY_FORWARD] && !keymap[KEY_BACKWARD]) {
        PlayerObj->speed[0] = 0;
        PlayerObj->speed[1] = 0;
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
    SDL_SetRenderDrawBlendMode(display->renderer, SDL_BLENDMODE_BLEND);
    display->last_frame_t = 0;
}

int main(void){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        exit(EXIT_FAILURE);
    }
    DisplaySettings display;
    init_display(&display);
    
    GameObject PlayerObj = {
        {64,64},    //pos
        {0,0},      //speed
        8,          //size
        2,          //angle
        0           //turn_speed
    };

    int mapgrid[] = {
        1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,
        0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,1,
        0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,
        1,1,0,0,1,1,1,1,0,0,0,1,0,0,0,1,
        1,0,0,0,0,0,0,0,0,2,2,1,1,1,0,1,
        1,0,0,0,0,0,1,0,0,0,0,3,0,0,0,1,
        1,0,1,0,0,1,1,1,0,0,0,3,0,0,0,1,
        1,1,1,0,1,1,1,1,0,0,0,0,0,0,0,1,
        1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,1,
        1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,1,0,1,0,0,1,0,2,0,3,0,1,
        1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,
        1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0
    };
    struct MapObj gameMap = {
        mapgrid,
        32,     //mapS
        16,     //mapX
        16      //mapY
    };

    while(running){
        process_input(&PlayerObj);
        update(&display, &PlayerObj, &gameMap);
        render_2d(&display, &PlayerObj, &gameMap);
    }
    destroy_display(&display);
    return 0;
}

