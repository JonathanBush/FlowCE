////////////////////////////////////////
// { FlowCE } { 0.1 }
// Author: Jonathan Bush
// License: GPL
// Description: Flow Free for the CE
////////////////////////////////////////

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
    uint8_t dim;
    uint8_t *board; //length of dim*dim
} flow_level_t;

    
/* function prototypes */
void displayTitleScreen(void);
flow_pack_t *loadPack(char *appvarName);
flow_pack_t *selectLevelPack();
flow_level_t *loadLevel(flow_pack_t *pack, uint8_t number);
uint8_t selectLevel(flow_pack_t *pack);
uint8_t playLevel(flow_level_t *level);
void drawNodes(flow_level_t *level);
void drawPipe(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t radius);
void drawCursor(uint8_t x, uint8_t y, uint8_t dim);
void fillCursor(uint8_t x, uint8_t y, uint8_t dim, uint8_t color);
void erasePipe(uint8_t x0, uint8_t y0, uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION], uint8_t dim);

void main(void) {
    uint8_t i, j, levelNum;
    flow_pack_t *selected;
    flow_level_t *level;
    
    gfx_Begin();
    
    // Use the custom colors
    gfx_SetPalette(flow_gfx_pal, sizeof(flow_gfx_pal), 0);

    //srand(rtc_Time());
    displayTitleScreen();
    selected = selectLevelPack();
    
    if (selected != NULL) {
        dbg_sprintf(dbgout, "Selected pack: %s\n", selected->name);
        levelNum = selectLevel(selected);
        level = loadLevel(selected, levelNum);
        playLevel(level);
        free(level->board);
        free(level);
    

    } else {
        dbg_sprintf(dbgout, "No packs\n");
    }
    delay(1500);
    
    while(!kb_AnyKey());
    free(selected->levelDimensions);
    free(selected->levelSizes);
    free(selected);
    gfx_End();
}

void displayTitleScreen() {
    uint8_t xPos, yPos, i;
    const uint8_t colors[6] = {FL_RED,FL_GREEN,FL_BLUE,FL_YELLOW,FL_WHITE,FL_WHITE};
    const char title[] = "FlowCE";

    gfx_FillScreen(FL_BLACK);
    gfx_SetTextScale(3, 5);
    xPos = (LCD_WIDTH - gfx_GetStringWidth(title)) / 2;
    yPos = (LCD_HEIGHT - 8*5) / 2 - 4*5;
    for(i = 0; i < strlen(title); i++) {
        gfx_SetTextFGColor(colors[i]);
        gfx_SetTextXY(xPos, yPos);
        gfx_PrintChar(title[i]);
        xPos += gfx_GetCharWidth(title[i]);
    }
    
    delay(TITLE_SCREEN_DELAY);
}


flow_pack_t * loadPack(char *appvarName) {
    unsigned char nameLength;
    uint8_t i;
    flow_pack_t *pack;
    ti_var_t packVar; 

    ti_CloseAll();
    
    packVar = ti_Open(appvarName, "r");
    dbg_sprintf(dbgout, "varname: %s\n", appvarName);

    if (!packVar) {
        dbg_sprintf(dbgout, "Appvar not opened\n");
        return NULL;
    }
    pack = (flow_pack_t*)malloc(sizeof(flow_pack_t));

    ti_Seek(4, SEEK_SET, packVar);    // skip the identifier
    ti_Read(&nameLength, 1, 1, packVar);
    ti_Read(pack->name, nameLength, 1, packVar);
    pack->name[nameLength] = '\0';
    ti_Read(&(pack->numLevels), 1, 1, packVar);

    dbg_sprintf(dbgout, "Pack name length: %d\n", nameLength);
    dbg_sprintf(dbgout, "Pack name: %s\n", pack->name);
    strcpy(pack->appvarName, appvarName);

    pack->levelSizes = malloc((size_t)pack->numLevels);
    pack->levelDimensions = malloc((size_t)pack->numLevels);

    for (i = 0; i < pack->numLevels; ++i) {
        ti_Read(&(pack->levelSizes[i]), 1, 1, packVar);
        ti_Read(&(pack->levelDimensions[i]), 1, 1, packVar);
    }
    
    ti_CloseAll();
    pack->levelOffset = 6 + nameLength + 2 * pack->numLevels;
    
    return pack;
}



flow_pack_t * selectLevelPack() {
    uint8_t *search_pos = NULL;
    const char search_string[] = "FLCE";
    uint8_t selection = 0;
    uint8_t numPacks = 0;
    kb_key_t key = 0xF0;
    uint8_t nameLength;
    char *var_name;
    char name[20];
    ti_var_t packVar;
    
    gfx_FillScreen(FL_BLACK);
    gfx_SetTextFGColor(FL_WHITE);
    gfx_SetTextScale(1, 1);
    
    ti_CloseAll();
    while ((var_name = ti_Detect(&search_pos, search_string)) != NULL) {
        
        packVar = ti_Open(var_name, "r");
        if (packVar != NULL) {
            ti_Seek(4, SEEK_SET, packVar);    // skip the identifier
            ti_Read(&nameLength, 1, 1, packVar);
            ti_Read(name, nameLength, 1, packVar);
            name[nameLength] = '\0';
            dbg_sprintf(dbgout, "Pack name: %s\n", name);
            gfx_PrintStringXY(name, 10, numPacks*PACK_SELECT_SPACING + 10);
            ++numPacks;
            ti_CloseAll();
        }
    }

    if (numPacks) {
        do {
            kb_Scan();
            
            if (key != kb_Data[7]) {
                key = kb_Data[7];
                
                gfx_SetColor(FL_BLACK);
                gfx_Rectangle_NoClip(5, selection*PACK_SELECT_SPACING + 5, 120, PACK_SELECT_SPACING);
                
                switch (key) {
                    case kb_Down:
                        selection = (selection + 1) % numPacks;
                        break;
                    case kb_Up:
                        selection = (selection) ? selection - 1 : numPacks - 1;
                        break;
                    default:
                        break;
                        
                }
                gfx_SetColor(FL_WHITE);
                gfx_Rectangle_NoClip(5, selection*PACK_SELECT_SPACING + 5, 120, PACK_SELECT_SPACING);
            }

        } while (kb_Data[1] != kb_2nd && kb_Data[6] != kb_Enter);
    } else {
        gfx_PrintStringXY("No packs available", 10, 10);
        return NULL;
    }
    dbg_sprintf(dbgout, "Selected pack: %d\n", selection);
    
    search_pos = NULL;
    
    for (; selection > 0; --selection) {
        ti_Detect(&search_pos, search_string);
    }
    var_name = ti_Detect(&search_pos, search_string);
    
    return loadPack(var_name);
}

flow_level_t * loadLevel(flow_pack_t *pack, uint8_t number) {
    //if (number >= pack->numLevels) return NULL;
    flow_level_t *level = malloc(sizeof(flow_level_t));
    ti_var_t packVar = ti_Open(pack->appvarName, "r");
    uint8_t node, color;
    uint16_t offset;
    
    if (packVar == NULL) {
        dbg_sprintf(dbgout, "Could not open \"%s\"\n", pack->appvarName);
    }

    
    level->dim = pack->levelDimensions[number];
    level->board = malloc(level->dim * level->dim);
    offset = pack->levelOffset;
    for (color = 0; color < number; ++color) {
        offset += pack->levelSizes[color];
    }
    
    ti_Seek(offset, SEEK_SET, packVar);    // skip the identifier
    for (color = 1; color <= pack->levelSizes[number] / 2; ++color) {    
        ti_Read(&node, 1, 1, packVar);
        level->board[node] = color;
        dbg_sprintf(dbgout, "Read node color %d at %d\n", color, node);
        ti_Read(&node, 1, 1, packVar);
        level->board[node] = color;
        dbg_sprintf(dbgout, "Read node color %d at %d\n", color, node);
    }
    
    ti_CloseAll();
    
    return level;
}

uint8_t selectLevel(flow_pack_t *pack) {
    uint8_t selection;
    uint8_t i;
    char levelText[4];
    kb_key_t key = 0xF0;
    
    gfx_SetDrawBuffer();
    gfx_FillScreen(FL_BLACK);
    gfx_SetTextFGColor(FL_WHITE);
    gfx_SetTextScale(1, 1);
    //gfx_SetColor(FL_WHITE);
    
    gfx_Blit(gfx_buffer);
    
    selection = 0;
    while (kb_AnyKey());
    
    dbg_sprintf(dbgout, "Starting selection\n");
    do {
        kb_Scan();
        
        if (key != kb_Data[7] /*&& (kb_Data[7] || key == 0xF0)*/) {
            dbg_sprintf(dbgout, "Update\n");
            //gfx_SetColor(FL_BLACK);
            //gfx_Rectangle_NoClip((selection % 5) * 48 + 1, ((selection / 5) % 5) * 48 + 1, 46, 46);
            key = kb_Data[7];
            switch (key) {
                case kb_Left:
                    selection -= (selection > 0);
                    break;
                case kb_Right:
                    selection += (selection + 1 < pack->numLevels);
                    break;
                case kb_Up:
                    selection -= 5*(selection - 5 >= 0);
                    break;
                case kb_Down:
                    selection += 5*(selection + 6 < pack->numLevels);
                    break;
                default:
                    break;
            }
            gfx_SetColor(FL_BLACK);
            gfx_FillRectangle_NoClip(1, 1, BOARD_SIZE, BOARD_SIZE);
            gfx_SetColor(FL_GRAY);
            gfx_FillRectangle_NoClip((selection % 5) * 48 + 1, ((selection / 5) % 5) * 48 + 1, 47, 47);
            gfx_SetColor(FL_WHITE);
            gfx_Rectangle_NoClip(0, 0, BORDER_SIZE, BORDER_SIZE);
            for (i = 0; i < BORDER_SIZE; i += 48) {
                gfx_Line_NoClip(1, i, BOARD_SIZE, i);
                gfx_Line_NoClip(i, 1, i, BOARD_SIZE);
            }
            //dbg_sprintf(dbgout, "bound: %d", ((25 * (selection / 25) + 25 < pack->numLevels) ? 25 * (selection / 25) + 25 : pack->numLevels));
            for (i = 25 * (selection / 25);
                    i < ((25 * (selection / 25) + 25 < pack->numLevels) ? 25 * (selection / 25) + 25 : pack->numLevels);
                    ++i) {
                sprintf(levelText, "%d", i + 1);
                gfx_SetTextXY((i % 5) * 48 + 24 - (gfx_GetStringWidth(levelText) / 2), ((i / 5) % 5) * 48 + 20);
                gfx_PrintString(levelText);
                //dbg_sprintf(dbgout, " %d", i);
            }
            gfx_SwapDraw();
            //dbg_sprintf(dbgout, "Selection: %d\n", selection);
        }
    } while (kb_Data[1] != kb_2nd);
    
    gfx_SetDrawScreen();
    return selection;
}

void drawNodes(flow_level_t *level) {
    uint8_t i, dim = level->dim;
    
    for (i = 0; i < dim * dim; ++i) {
        uint8_t bz = BOARD_SIZE / (2 * dim);
        uint8_t node = level->board[i];
        
        if (node) {
            dbg_sprintf(dbgout, "Node %u\n", node);
            gfx_SetColor(node);
            gfx_FillCircle_NoClip(
                bz + (((i % dim) * BOARD_SIZE) / dim),
                bz + (((i / dim) * BOARD_SIZE) / dim),
                (3 * bz) / 4
            );
        }
    }
}

/* return 0 if incomplete, 1 if complete, 2 if perfect */
uint8_t playLevel(flow_level_t *level) {
    uint8_t i, x, y, x0, y0;
    uint8_t boardSize, exit, selection;
    uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION];
    uint8_t dim = level->dim;
    kb_key_t key;
    
    gfx_SetDrawBuffer();
    gfx_FillScreen(FL_BLACK);
    gfx_SetTextFGColor(FL_WHITE);
    gfx_SetTextScale(1, 1);
    gfx_SetColor(FL_BORDER_COLOR);
    
    for (i = 0; i <= level->dim; ++i) {
        uint8_t pos = (i * BOARD_SIZE) / level->dim;
        gfx_Line_NoClip(0, pos, BOARD_SIZE, pos);
        gfx_Line_NoClip(pos, 0, pos, BOARD_SIZE);
    }
    for (x = 0; x < dim; ++x) {
        for (y = 0; y < dim; ++y) {
            board[x][y] = level->board[x + y * dim];
        }
    }
    x = y = 0;
    selection = 0;

    drawNodes(level);
    
    while(kb_AnyKey());
    
    // game loop:
    exit = 0;
    fillCursor(x, y, level->dim, FL_CURSOR_COLOR);
    do {
        //gfx_SetColor(FL_BORDER_COLOR);
        //drawCursor(x, y, level->dim);
        
        gfx_Blit(gfx_buffer);
        
        
        do {
            kb_Scan();
        } while (
                !kb_Data[7] &&
                kb_Data[6] != kb_Clear &&
                kb_Data[6] != kb_Enter &&
                kb_Data[1] != kb_2nd
            );
            
        dbg_sprintf(dbgout, "Key pressed\n");    
        if (kb_Data[6] == kb_Clear) {
            // quit button pressed
            exit = 1;
            continue;
        } else if (kb_Data[6] == kb_Enter || kb_Data[1] == kb_2nd) {
            // selection button pressed
            if (selection) {
                selection = 0;
            } else if (board[x][y]) {
                selection = board[x][y] & 0x00FF;
                if ((board[x][y] & 0xFF00) == 0x00) {  // this is the first node
                dbg_sprintf(dbgout, "First node selected\n");
                    board[x][y] |= 0x0100;
                    for (x0 = 0; x0 < dim; ++x0) {
                        for (y0 = 0; y0 < dim; ++y0) {
                            if (board[x0][y0] == selection) { // this is the other node
                                board[x0][y0] |= 0xFF00;
                                x0 = 250;
                                break;
                            }
                        }
                    }
                } else if ((board[x][y] & 0xFF00) == 0xFF00) {  // the wrong node was selected
                    dbg_sprintf(dbgout, "Wrong node selected\n");
                    for (x0 = 0; x0 < dim; ++x0) {
                        for (y0 = 0; y0 < dim; ++y0) { // find the first node
                            if (board[x0][y0] == (0x0100 | selection)) {
                                erasePipe(x0, y0, board, dim);
                                drawNodes(level);
                                board[x0][y0] = 0xFF00 | selection;
                                x0 = 250;
                                break;
                            }
                        }
                    }
                    board[x][y] = 0x0100 | selection;
                }
            }
            
            
            dbg_sprintf(dbgout, "Selection: %d\n", selection);
        } else if (kb_Data[7]) {
            // arrow key pressed
            dbg_sprintf(dbgout, "Arrow key %u\n", kb_Data[7]);
            fillCursor(x, y, level->dim, FL_BLACK);
            x0 = x;
            y0 = y;
            switch (kb_Data[7]) {
                case kb_Left:
                    x -= (x > 0);
                    break;
                case kb_Right:
                    x += (x + 1 < level->dim);
                    break;
                case kb_Up:
                    y -= (y > 0);
                    break;
                case kb_Down:
                    y += (y + 1 < level->dim);
                    break;
                default:
                    break;
            }
            
            if ((x0 != x || y0 != y) && selection) {
                gfx_SetColor(selection);
                if (!board[x][y]) { // nothing there yet
                    dbg_sprintf(dbgout, "Moved to (%d, %d)\n", x, y);
                    drawPipe(x0, y0, x, y, dim);
                    board[x][y] = selection | ((((board[x0][y0] & 0xFF00) >> 8) + 1) << 8);
                } else if (board[x][y] & 0xFF == selection) { // this color already there
                
                } else { // some other color already there
                
                }
                
                
            }
            fillCursor(x, y, level->dim, FL_CURSOR_COLOR);
        }
        
        //gfx_SetColor(FL_BLACK);
        //drawCursor(x, y, level->dim);
        
        while (kb_AnyKey());
        
    } while (!exit);

    gfx_SetColor(FL_BORDER_COLOR);
    
    

    gfx_Blit(gfx_buffer);

    return 0;
}

/* draw a pipe segment from (x0, y0) to (x1, y1) */
void drawPipe(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t dim) {
    uint8_t radius = BOARD_SIZE / (6 * dim);
    uint8_t bz = BOARD_SIZE / (2 * dim);

    x0 = (x0 * BOARD_SIZE) / dim + bz;
    y0 = (y0 * BOARD_SIZE) / dim + bz;
    x1 = (x1 * BOARD_SIZE) / dim + bz;
    y1 = (y1 * BOARD_SIZE) / dim + bz;
    
    gfx_FillCircle_NoClip(x0, y0, radius);
    gfx_FillCircle_NoClip(x1, y1, radius);
    if (x0 == x1) {
        if (y0 < y1) {
            gfx_FillRectangle_NoClip(x0 - (radius), y0, radius * 2 + 1, y1 - y0);
        } else {
            gfx_FillRectangle_NoClip(x0 - (radius), y1, radius * 2 + 1, y0 - y1);
        }
    } else {
        if (x0 < x1) {
            gfx_FillRectangle_NoClip(x0, y0 - (radius), x1 - x0, radius * 2 + 1);
        } else {
            gfx_FillRectangle_NoClip(x1, y0 - (radius), x0 - x1, radius * 2 + 1);
        }
    }
}

void drawCursor(uint8_t x, uint8_t y, uint8_t dim) {
    uint8_t x0, x1, x2, x3,
            y0, y1, y2, y3,
            length;
    x0 = (x * BOARD_SIZE) / dim + 1;
    y0 = (y * BOARD_SIZE) / dim + 1;
    x1 = ((x + 1) * BOARD_SIZE) / dim - 1;
    y1 = ((y + 1) * BOARD_SIZE) / dim - 1;
    length = BOARD_SIZE / (5 * dim);
    x2 = x0 + length;
    y2 = y0 + length;
    x3 = x1 - length;
    y3 = y1 - length;
    
    gfx_Line_NoClip(x0, y0, x2, y2);
    gfx_Line_NoClip(x1, y0, x3, y2);
    gfx_Line_NoClip(x1, y1, x3, y3);
    gfx_Line_NoClip(x0, y1, x2, y3);
}

void fillCursor(uint8_t x, uint8_t y, uint8_t dim, uint8_t color) {
    uint8_t x0 = (x * BOARD_SIZE) / dim + 1;
    uint8_t y0 = (y * BOARD_SIZE) / dim + 1;
    gfx_FloodFill(x0, y0, color);
}
        
void erasePipe(uint8_t x, uint8_t y, uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION], uint8_t dim) {
    uint8_t x1, y1, exit;
    uint8_t color = board[x][y] & 0x00FF;
    uint16_t count = board[x][y] & 0xFF00;
    
    x1 = x;
    y1 = y;
    dbg_sprintf(dbgout, "erasePipe(%d, %d,...); count=%u, color=%u\n", x, y, count, color);
    for (exit = 1; exit;) {
        count = (1 + (count >> 8)) << 8;
   
        if        (x+1 < dim && board[x+1][y] == (count | color)) {
            ++x1;
        } else if (y+1 < dim && board[x][y+1] == (count | color)) {
            ++y1;
        } else if (x-1 >= 0  && board[x-1][y] == (count | color)) {
            --x1;
        } else if (y-1 >= 0  && board[x][y-1] == (count | color)) {
            --y1;
        }
        if (x1 != x || y1 != y) {
            uint8_t xMax = max(x, x1);
            uint8_t yMax = max(y, y1);
            gfx_SetColor(FL_BLACK);
            drawPipe(x, y, x1, y1, dim);    // erase the pipe
            dbg_sprintf(dbgout, "Erased segment from (%d, %d) to (%d, %d)\n", x, y, x1, y1);
            gfx_SetColor(FL_BORDER_COLOR);
            if (x != x1) {  // movement in the x direction
                uint8_t yTop = (yMax * BOARD_SIZE) / dim;
                yMax = ((yMax + 1) * BOARD_SIZE) / dim;
                xMax = (xMax * BOARD_SIZE) / dim;
                gfx_Line_NoClip(xMax, yTop, xMax, yMax);
            } else { // movement in the y direction
                uint8_t xLeft = ((xMax) * BOARD_SIZE) / dim;
                yMax = (yMax * BOARD_SIZE) / dim;
                xMax = ((xMax + 1) * BOARD_SIZE) / dim;
                gfx_Line_NoClip(xLeft, yMax, xMax, yMax);
            }
            board[x1][y1] = 0;  // remove from the board
            x = x1;
            y = y1;
        } else {
            exit = 0;
        } 
    }
    
}