#ifndef CONFIG_H
#define CONFIG_H
#define PLAYER_SPEED 5
#define TURN_SPEED 0.08
#define PI 3.14159265359
#define RAY_NUMBER 241                 //must be odd to have simetry
#define FOV_ANGLE PI/3.0
#define DIR_VEC_SIZE 20.00
#define ROTATE_ANGLE FOV_ANGLE/(RAY_NUMBER)
#define CAST_3D_OFFSET 512/((int)RAY_NUMBER)
#define FPS 60                          //frequencia
#define FRAME_TARGET_TIME (1000 / FPS)  //Periodo (em ms)

#endif
