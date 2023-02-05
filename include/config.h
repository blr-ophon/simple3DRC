#ifndef CONFIG_H
#define CONFIG_H

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 512
#define GAME_WIDTH 512
#define GAME_HEIGHT GAME_WIDTH/2
#define GAME_X 512
#define GAME_Y 256

#define FPS 60                          //frequencia
#define FRAME_TARGET_TIME (1000 / FPS)  //Periodo (em ms)
                                        //
#define PLAYER_SPEED 2
#define TURN_SPEED 0.05
#define DIR_VEC_SIZE 30

#define RAY_NUMBER 121                 //must be odd to have simetry
#define PI 3.14159265359
#define FOV_ANGLE PI/3.0
#define UNIT_ANGLE FOV_ANGLE/(RAY_NUMBER)
#define CAST_3D_OFFSET GAME_WIDTH/((int)RAY_NUMBER)

#endif
