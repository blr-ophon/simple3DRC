#include "textures.h"
#include "raycasting.h"

/*
int textureX = 8, textureY = 8;

int texture[][8] = {
    {0,0,0,3,3,0,0,0},
    {0,0,3,3,3,3,0,0},
    {0,3,2,3,3,2,3,0},
    {0,3,3,3,3,3,3,0},
    {0,3,2,3,3,2,3,0},
    {0,3,3,2,2,3,3,0},
    {0,0,3,3,3,3,0,0},
    {0,0,0,3,3,0,0,0}
};
*/
int textureY = 12, textureX = 12;

int texture[][12] = {
    {0,0,0,3,3,0,0,0,3,2,3,2},
    {0,0,3,3,3,3,0,0,2,3,2,3},
    {0,3,2,3,3,2,3,0,3,2,3,2},
    {0,3,3,3,3,3,3,0,2,3,2,3},
    {0,3,2,3,3,2,3,0,3,2,3,2},
    {0,3,3,2,2,3,3,0,2,3,2,3},
    {0,0,3,3,3,3,0,0,3,2,3,2},
    {0,0,0,3,3,0,0,0,2,3,2,3},
    {0,0,0,3,3,0,0,0,3,2,3,2},
    {0,0,3,3,3,3,0,0,2,3,2,3},
    {0,3,2,3,3,2,3,0,3,2,3,2},
    {0,3,3,3,3,3,3,0,2,3,2,3}
};

void BMPtoArray(SDL_Color *colorsMap[64]){
    FILE *f = fopen("test.bmp", "rb");

    uint8_t info[54];
    fread(info, sizeof(uint8_t), 54, f);

    uint32_t start_offset = info[10];
    uint32_t width = info[18];
    uint32_t height = info[22];
    uint16_t bpp = info[28];
    uint32_t compression = info[30];

    printf("WIDTH: %d\n", width);
    printf("HEIGHT: %d\n", height);
    printf("BPP:%d\n", bpp);
    printf("COMPRESSION: %d\n", compression);

    fseek(f, start_offset, SEEK_SET);
    for(uint32_t i = 0; i < width; i++){ //FOLLOWING LOOP WORKS FOR BPP = 24
        for(uint32_t j = 0; j < height*4; j+=4){
            fread(&colorsMap[i][j/4].b, 1, 1, f);
            fread(&colorsMap[i][j/4].g, 1, 1, f);
            fread(&colorsMap[i][j/4].r, 1, 1, f);
        }
    }
}

void getRayTexture(struct ray_object *castedRay, float *VectorDir, int *mapgrid, int mapX, int mapS){
    int mapCollum = (castedRay->endP[0] / mapS);
    int mapLine = (castedRay->endP[1] / mapS);
    int WallColorOffset; 

    if(castedRay->horizontal){
        WallColorOffset = VectorDir[0] < 0? -1 : 0;
        mapCollum += WallColorOffset;
        //Equation: (x%mapS)/mapS  =  textureXindex/textureX
        castedRay->textureXindex = (fmod(castedRay->endP[1], mapS) / mapS) * textureX;
    }else{ 
        WallColorOffset = VectorDir[1] < 0? -1 : 0;
        mapLine += WallColorOffset;
        //Equation: (x%mapS)/mapS  =  textureXindex/textureX
        castedRay->textureXindex = (fmod(castedRay->endP[0], mapS) / mapS) * textureX;
    }
    castedRay->colorIndex = mapgrid[mapX*(mapLine) + mapCollum];
}

void DrawTextures(SDL_Renderer *renderer, struct ray_object *castedRay, float lineH, float lineY, int CollumX){
    for(int i = 0; i < textureY; i++){
        int colorIndex = texture[i][castedRay->textureXindex];

        SDL_Color color = IndexToColor(colorIndex);
        if(castedRay->horizontal) shadeColor(&color);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

        SDL_Rect GameCollumRender = {CollumX, lineY, CAST_3D_OFFSET, lineH/textureY + 1};
        SDL_RenderFillRect(renderer, &GameCollumRender);
        lineY += lineH/textureX; 
    }
}

SDL_Color IndexToColor(int Index){
    SDL_Color color;
    switch(Index){
        case 0: 
            color.r = 0;
            color.g = 0;
            color.b = 155;
            break;
        case 1:
            color.r = 0;
            color.g = 0;
            color.b = 155;
            break;
        case 2: 
            color.r = 5;
            color.g = 5;
            color.b = 5;
            break;
        case 3: 
            color.r = 155;
            color.g = 155;
            color.b = 155;
            break;
    }
    return color;
}

void shadeColor(SDL_Color *color){
    //TODO: make shading factor a constant
    color->r /= 3;
    color->g /= 3;
    color->b /= 3;
}
