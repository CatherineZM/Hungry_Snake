//
//  objects.h
//  
//
//  Created by 周沫 on 2020-04-04.
//

#ifndef objects_h
#define objects_h

#include<stdbool.h>

typedef struct snakeHead{
    int x;
    int y;
    int image_W;
    int image_H;
    int head_type; //0 for up, 1 for dpwn, 2 for right, 3 for left
} snakeHead;

typedef struct Body{
    int x;
    int y;
    int image_W;
    int image_H;
}Body;

typedef struct fruit{
    int x;
    int y;
    int image_W;
    int image_H;
    int fruit_type; //0 for apple, 1 for strawberry
}fruit;

typedef struct Snakes
{
    int x;
    int y;
    struct Snakes *next;
}snake;


int *fruitSelection[2] = {image_apple_24x24, image_straw_24x24};

int *headSelection[4] = {image_UP_24x24, image_DOWN_24x24, image_RIGHT_24x24, image_LEFT_24x24};

#endif /* objects_h */
