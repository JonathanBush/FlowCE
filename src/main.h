#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>
#include <debug.h>

#include "flow_colors.h"

#define TITLE_SCREEN_DELAY (1400)
#define BORDER_SIZE (240)
#define BOARD_SIZE (BORDER_SIZE - 2)
#define PACK_SELECT_SPACING (18)
#define MAX_BOARD_DIMENSION (14)
#define KEY_REPEAT_DELAY (100)
#define statusX (244)
#define statusY (40)
#define titleY (2)
#define statusSpace (16)

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

typedef struct flow_pack_t {
    char name[20];
    char appvarName[9];
    uint8_t levelOffset;
    uint8_t numLevels;
    uint8_t *levelDimensions;    // dimensions of level board
    uint8_t *levelSizes;         // lengths of level data in bytes
} flow_pack_t;

typedef struct flow_level_t {
    uint8_t number;
    uint8_t dim;
    uint8_t flows;
    uint8_t pipe;
    uint8_t *board; //length of dim*dim
} flow_level_t;

typedef struct path_point {
    uint8_t x;
    uint8_t y;
    struct path_point *next;
} path_point;

    
/* function prototypes */
void displayTitleScreen(void);

uint8_t endMenu(char *title, char **options, uint8_t num, uint8_t mask);
void polygonXY_NoClip(uint8_t *points, unsigned num_points, uint24_t x, uint8_t y);



/* global variables */
extern uint16_t pipeComplete;
extern uint8_t flowsComplete;
extern uint8_t colorsComplete[20];
extern path_point *pathMemory[24];



#endif