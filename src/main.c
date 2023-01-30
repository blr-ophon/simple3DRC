#include "main.h"
#include <stdbool.h>
#include <assert.h>
#include <math.h>

bool running = true;
GameObject PlayerObj = {
    100,    //x
    300,    //y
    0,      //x_speed
    0,      //y_speed
    8,      //size
    0,      //angle
    0       //turn_speed
};

int last_frame_time = 0;

int mapX = 8, mapY = 8, mapS = 64;
int mapgrid[][8] = {
    {1,1,1,1,1,1,1,1},
    {1,0,0,0,1,1,1,1},
    {1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,0,1},
    {1,0,0,0,1,0,1,1},
    {1,0,0,0,0,0,1,1},
    {1,0,1,1,1,0,1,1},
    {1,1,1,1,1,1,1,1}
};

bool IsColliding(int x, int y){
    return mapgrid[y/mapS][x/mapS];
}


void render_2d(DisplaySettings *display){
    SDL_SetRenderDrawColor(display->renderer, 50, 50, 50, 255);
    SDL_RenderClear(display->renderer);

    //grid
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            SDL_Rect GameWalls = {i*mapS, j*mapS, mapS-1, mapS-1};
            SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
            if(mapgrid[j][i]){
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
    float point_E[2];
    float VecDSize = 30;
    point_E[0] = PlayerObj.x + VecDSize*cos(PlayerObj.angle);
    point_E[1] = PlayerObj.y + VecDSize*sin(PlayerObj.angle);
    SDL_SetRenderDrawColor(display->renderer, 0, 155, 0, 255);
    SDL_RenderDrawLine(display->renderer, PlayerObj.x, PlayerObj.y, point_E[0], point_E[1]);

    //vector d
    float VectorDir[] = {point_E[0]-PlayerObj.x, point_E[1]-PlayerObj.y};
    
    //Main ray and points
    float VecMainRay[] = {VectorDir[0],VectorDir[1]};
    /*
    //first point P1, X
    float VecMainRayX[] = {VectorDir[0],VectorDir[1]};
    float P1Ratio = 0;
    if(VectorDir[0] != 0){
        int delta_X1 = VectorDir[0] > 0? mapS - (((int)PlayerObj.x)%mapS) : ((int)PlayerObj.x)%mapS;
        P1Ratio = delta_X1/fabs(VectorDir[0]);
    }
    VecMainRayX[0] *= P1Ratio;
    VecMainRayX[1] *= P1Ratio;
    float VecMainRayXsize = P1Ratio*VecDSize;
    */

    //first point P1, Y
    float VecMainRayY[] = {VectorDir[0],VectorDir[1]};
    float P1Ratio = 0;
    if(VectorDir[1] != 0){
        int delta_Y1 = VectorDir[1] > 0? mapS - (((int)PlayerObj.y)%mapS) : ((int)PlayerObj.y)%mapS;
        P1Ratio = delta_Y1/fabs(VectorDir[1]);
    }
    VecMainRayY[0] *= P1Ratio;
    VecMainRayY[1] *= P1Ratio;
    float VecMainRayYsize = P1Ratio*VecDSize;

    //logic to determine if collision happens in X or Y vector
    VecMainRay[0] = VecMainRayY[0];
    VecMainRay[1] = VecMainRayY[1];
    
    SDL_SetRenderDrawColor(display->renderer, 0, 0, 155, 255);
    SDL_RenderDrawLine(display->renderer, PlayerObj.x, PlayerObj.y, PlayerObj.x+VecMainRay[0], PlayerObj.y+VecMainRay[1]);
    SDL_RenderPresent(display->renderer);
}

void update(void){
    int time_passed = SDL_GetTicks() - last_frame_time;
    int time_to_wait = FRAME_TARGET_TIME - time_passed;
    if(time_to_wait > 0){
        SDL_Delay(time_to_wait);
    }

    //TODO: Implement movement as delta time function
    //(x,y) movement
    if(!IsColliding(PlayerObj.x + PlayerObj.x_speed, PlayerObj.y + PlayerObj.y_speed)){
        PlayerObj.x += PlayerObj.x_speed;
        PlayerObj.y += PlayerObj.y_speed;
    }
    //view direction movement
    PlayerObj.angle += PlayerObj.turn_speed; 
    last_frame_time = SDL_GetTicks();
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

int main(void){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        exit(EXIT_FAILURE);
    }
    DisplaySettings display;
    init_display(&display);
    while(running){
        process_input();
        update();
        render_2d(&display);
    }
    return 0;
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
}
