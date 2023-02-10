#include "textures.h"
#include "raycasting.h"

TextureMap *BMPtoArray(void){
    TextureMap *texture = malloc(sizeof(TextureMap));
    FILE *f = fopen("./textures/test.bmp", "rb");
    if(!f){
        printf("Texture not found\n\n");
        exit(EXIT_FAILURE);
    }

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

    SDL_Color *pMap = malloc(width*height*sizeof(SDL_Color));

    fseek(f, start_offset, SEEK_SET);
    for(uint32_t i = 0; i < width; i++){ //FOLLOWING LOOP WORKS FOR BPP = 24
        for(uint32_t j = 0; j < height*4; j+=4){
            fread(&pMap[i*width + j/4].b, 1, 1, f);
            fread(&pMap[i*width + j/4].g, 1, 1, f);
            fread(&pMap[i*width + j/4].r, 1, 1, f);
        }
    }
    fclose(f);

    texture->pixelMap = pMap;
    texture->height = height;
    texture->width = width;
    return texture;
}

void getRayTexture(struct ray_object *castedRay, float *VectorDir, int *mapgrid, int mapX, int mapS, TextureMap *texture){
    int mapCollum = (castedRay->endP[0] / mapS);
    int mapLine = (castedRay->endP[1] / mapS);
    int WallColorOffset; 

    if(castedRay->horizontal){
        WallColorOffset = VectorDir[0] < 0? -1 : 0;
        mapCollum += WallColorOffset;
        //Equation: (x%mapS)/mapS  =  textureXindex/textureWidth
        castedRay->textureXindex = (fmod(castedRay->endP[1], mapS) / mapS) * texture->width;
    }else{ 
        WallColorOffset = VectorDir[1] < 0? -1 : 0;
        mapLine += WallColorOffset;
        //Equation: (x%mapS)/mapS  =  textureXindex/textureWidth
        castedRay->textureXindex = (fmod(castedRay->endP[0], mapS) / mapS) * texture->width;
    }
    //TODO: this is now texture type index
    castedRay->colorIndex = mapgrid[mapX*(mapLine) + mapCollum];
}

void DrawTextures(SDL_Renderer *renderer, struct ray_object *castedRay, float lineH, float lineY, int CollumX, TextureMap *texture){
    for(int i = 0; i < texture->height; i++){
        //int colorIndex = texture[i][castedRay->textureXindex];

        //SDL_Color color = IndexToColor(colorIndex);
        SDL_Color color = texture->pixelMap[i*texture->width + castedRay->textureXindex];
        if(castedRay->horizontal) shadeColor(&color);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

        SDL_Rect GameCollumRender = {CollumX, lineY, CAST_3D_OFFSET, lineH/texture->height + 1};
        SDL_RenderFillRect(renderer, &GameCollumRender);
        lineY += lineH/texture->width; 
    }
}

void shadeColor(SDL_Color *color){
    //TODO: make shading factor a constant
    color->r /= 3;
    color->g /= 3;
    color->b /= 3;
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

