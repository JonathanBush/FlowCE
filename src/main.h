#ifndef MAIN_H
#define MAIN_H

#define TITLE_SCREEN_DELAY (1400)
#define BORDER_SIZE (240)
#define BOARD_SIZE (BORDER_SIZE - 2)
#define PACK_SELECT_SPACING (18)
#define MAX_BOARD_DIMENSION (14)

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
flow_pack_t *loadPack(char *appvarName);
flow_pack_t *selectLevelPack();
flow_level_t *loadLevel(flow_pack_t *pack, uint8_t number);
int selectLevel(flow_pack_t *pack, uint8_t *progress);
uint8_t playLevel(flow_level_t *level);
uint8_t endMenu(char *title, char **options, uint8_t num, uint8_t mask);

void drawNodes(flow_level_t *level);
void drawPipe(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t radius);
void drawCursor(uint8_t x, uint8_t y, uint8_t dim);
void fillCursor(uint8_t x, uint8_t y, uint8_t dim, uint8_t color);

void polygonXY_NoClip(uint8_t *points, unsigned num_points, uint24_t x, uint8_t y);

uint8_t *loadProgress(flow_pack_t *pack);
void saveProgress(flow_pack_t *pack, uint8_t *progress);
void erasePipe(uint8_t x0, uint8_t y0, uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION], flow_level_t *level);
void erasePipeFrom(uint16_t key, uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION], flow_level_t *level);
void clearPathMemory();
void clearLinkedList(path_point *head);
uint8_t scanPathMemory(uint8_t x, uint8_t y, uint8_t ignoreColor);
void restorePipe(uint8_t color, uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION], flow_level_t *level);

/* global variables */
uint16_t pipeComplete;
uint8_t flowsComplete;
uint8_t colorsComplete[20];
path_point *pathMemory[24];
char zeroID[10] = {0,0,0,0,0,0,0,0,0,0};
const uint24_t statusX = 244;
const uint8_t statusY = 40;
const uint8_t titleY = 2;
const uint8_t statusSpace = 16;

#endif