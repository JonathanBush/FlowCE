////////////////////////////////////////
// { FlowCE } { 0.1 }
// Author: Jonathan Bush
// License: GPL
// Description: Flow Free for the CE
////////////////////////////////////////

/* Keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
/* Standard headers - it's recommended to leave them included */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Other available headers: stdarg.h, setjmp.h, assert.h, ctype.h, float.h, iso646.h, limits.h, errno.h */
#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>
#include <debug.h>

#include "flow_colors.h"

#define TITLE_SCREEN_DELAY (1400)
#define BORDER_SIZE (240)
#define BOARD_SIZE (BORDER_SIZE - 2)
#define PACK_SELECT_SPACING (18)


typedef struct flow_pack_t {
    char name[20];
    char appvarName[9];
    uint8_t levelOffset;
    uint8_t numLevels;
    uint8_t *levelDimensions;    // dimensions of level board
    uint8_t *levelSizes;        // lengths of level data in bytes
} flow_pack_t;

typedef struct flow_level_t {
    uint8_t dim;
    uint8_t *board; //length of dim*dim
} flow_level_t;

    
/* function prototypes */
void displayTitleScreen(void);
flow_pack_t * loadPack(char *appvarName);
flow_pack_t * selectLevelPack();
flow_level_t * loadLevel(flow_pack_t *pack, uint8_t number);
uint8_t selectLevel(flow_pack_t *pack);
uint8_t playLevel(flow_level_t *level);

void main(void) {
    uint8_t i, j, levelNum;
    flow_pack_t *selected;
    flow_level_t *level;
    /* This function cleans up the screen and gets everything ready for the OS */
    pgrm_CleanUp();
    gfx_Begin( gfx_8bpp );
    //gfx_SetMonospaceFont(false);
    gfx_SetPalette(flow_gfx_pal, 288, 0);   // Use the custom colors
    // Do all the stuff here
    //srand(rtc_Time());
    displayTitleScreen();
    selected = selectLevelPack();
    
    if (selected != NULL) {
        dbg_sprintf(dbgout, "Selected pack: %s", selected->name);
        levelNum = selectLevel(selected);
        level = loadLevel(selected, levelNum);
        playLevel(level);
        free(level->board);
        free(level);
    
    //displayTitleScreen();
    } else {
        dbg_sprintf(dbgout, "No packs\n");
    }
    delay(1500);
    
    
    while(!kb_AnyKey());
    free(selected->levelDimensions);
    free(selected->levelSizes);
    free(selected);
    gfx_End();
    pgrm_CleanUp();
}

void displayTitleScreen() {
    uint8_t xPos, yPos, i;
    const uint8_t colors[10] = {FL_RED,FL_GREEN,FL_BLUE,FL_YELLOW,FL_WHITE,FL_WHITE};
    const char title[] = "FlowCE";
    char single[2];

    gfx_FillScreen(FL_BLACK);
    gfx_SetTextScale(3, 5);
    xPos = (LCD_WIDTH - gfx_GetStringWidth(title)) / 2;
    yPos = (LCD_HEIGHT - 8*5) / 2 - 4*5;
    for(i = 0; i < strlen(title); i++) {
        gfx_SetTextFGColor(colors[i]);
        sprintf(single, "%c", title[i]);
        gfx_PrintStringXY(single, xPos, yPos);
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
    dbg_sprintf(dbgout, "here1");
    packVar = ti_Open(appvarName, "r");
    dbg_sprintf(dbgout, "varname: %s\n", appvarName);

    
    if (!packVar) {
        dbg_sprintf(dbgout, "Appvar not opened\n");
        return NULL;
    }
    pack = (flow_pack_t *)malloc(sizeof(flow_pack_t));
    
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
        dbg_sprintf(dbgout, "here2");
        
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
        dbg_sprintf(dbgout, "Could not open \"%s\"", pack->appvarName);
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
    while(kb_AnyKey());
    
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
            dbg_sprintf(dbgout, "bound: %d", ((25 * (selection / 25) + 25 < pack->numLevels) ? 25 * (selection / 25) + 25 : pack->numLevels));
            for (i = 25 * (selection / 25);
                    i < ((25 * (selection / 25) + 25 < pack->numLevels) ? 25 * (selection / 25) + 25 : pack->numLevels);
                    ++i) {
                sprintf(levelText, "%d", i + 1);
                gfx_SetTextXY((i % 5) * 48 + 24 - (gfx_GetStringWidth(levelText) / 2), ((i / 5) % 5) * 48 + 20);
                gfx_PrintString(levelText);
                //dbg_sprintf(dbgout, " %d", i);
            }
            gfx_SwapDraw();
            dbg_sprintf(dbgout, "Selection: %d\n", selection);
        }
    } while (kb_Data[1] != kb_2nd);
    
    gfx_SetDrawScreen();
    return selection;
}

void drawNodes(flow_level_t *level) {
    uint8_t i;
    for (i = 0; i < level->dim * level->dim; ++i) {
        if (level->board[i]) {
            dbg_sprintf(dbgout, "Node %d\n", level->board[i]);
            gfx_SetColor(level->board[i]);
            gfx_FillCircle_NoClip(
                ((BOARD_SIZE) / (2 * level->dim)) + (((i % level->dim) * (BOARD_SIZE)) / level->dim),
                ((BOARD_SIZE) / (2 * level->dim)) + (((i / level->dim) * (BOARD_SIZE)) / level->dim),
                (BOARD_SIZE) / (2 * level->dim) - 1
            );
        }
    }
}

/* return 0 if incomplete, 1 if complete, 2 if perfect */
uint8_t playLevel(flow_level_t *level) {
    gfx_SetDrawBuffer();
    gfx_FillScreen(FL_BLACK);
    gfx_SetTextFGColor(FL_WHITE);
    gfx_SetTextScale(1, 1);
    gfx_SetColor(FL_WHITE);
    
    drawNodes(level);
    
    gfx_Blit(gfx_buffer);
    
    return 0;
    
}
    