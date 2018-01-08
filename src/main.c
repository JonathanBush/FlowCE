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

#include "main.h"


void main(void) {
    uint8_t i, j;
    int levelNum;
    flow_pack_t *selected;
    flow_level_t *level;
    uint8_t *progress;
    
    gfx_Begin();
    
    // Use the custom colors
    gfx_SetPalette(flow_gfx_pal, sizeof(flow_gfx_pal), 0);

    //srand(rtc_Time());
    displayTitleScreen();
    selected = selectLevelPack();
    
    if (selected != NULL) {
        uint8_t selection;
        char *perfectText = "Perfect!";
        char *completeText = "Complete!";
        char *optionsText = "Options";
        char *options[4];

        options[0] = "Next Level";
        options[1] = "Try Again";
        options[2] = "Select Level";
        options[3] = "Quit";

        progress = loadProgress(selected);
        levelNum = selectLevel(selected, progress, 0);
        
        while (levelNum != -1) {
            level = loadLevel(selected, levelNum);

            switch (playLevel(level)) {
                case 1:
                    selection = endMenu(completeText, options, 4, 0x0F - (levelNum + 1 >= selected->numLevels));
                    progress[levelNum] |= 0x1;
                    break;
                case 2:
                    selection = endMenu(perfectText,  options, 4, 0x0D - (levelNum + 1 >= selected->numLevels));
                    progress[levelNum] |= 0x3;
                    break;
                default:
                    selection = endMenu(optionsText, options, 4, 0x0E);
                    //selection = 3;
                    break;
                    
            }
            switch (selection) {
                case 0:
                    ++levelNum;
                    break;
                case 1:
                    // play the level again
                    break;
                case 2:
                    levelNum = selectLevel(selected, progress, levelNum);
                    break;
                case 3:
                    levelNum = -1;
                    break;
            }
            free(level->board);
            free(level);
            clearPathMemory();
        }
        saveProgress(selected, progress);
        free(progress);
    } else {
        dbg_sprintf(dbgout, "No packs\n");
    }
    //delay(1500);
    
    while(kb_AnyKey());
    
    if (selected != NULL) {
        free(selected->levelDimensions);
        free(selected->levelSizes);
        free(selected);
    }
    gfx_End();
    os_ClrHome();
}

void displayTitleScreen() {
    uint8_t xPos, yPos, i;
    const uint8_t colors[6] = {FL_RED,FL_GREEN,FL_BLUE,FL_YELLOW,FL_WHITE,FL_WHITE};
    const char title[] = "FlowCE";
    const char author[] = "by jonbush";

    gfx_FillScreen(FL_BLACK);
    gfx_SetTextScale(1,1);
    gfx_SetTextFGColor(FL_WHITE);
    gfx_PrintStringXY(author, (LCD_WIDTH - gfx_GetStringWidth(author)) / 2, 150);
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

    if (!packVar) {
        return NULL;
    }
    pack = (flow_pack_t*)malloc(sizeof(flow_pack_t));

    ti_Seek(4, SEEK_SET, packVar);    // skip the identifier
    ti_Read(&nameLength, 1, 1, packVar);
    ti_Read(pack->name, nameLength, 1, packVar);
    pack->name[nameLength] = '\0';
    ti_Read(&(pack->numLevels), 1, 1, packVar);

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
    gfx_SetTextScale(1, 2);
    gfx_PrintStringXY("FlowCE", statusX, titleY);
    gfx_SetTextScale(1, 1);
    gfx_PrintStringXY("Select", statusX, statusY);
    gfx_PrintStringXY("Pack", statusX, statusY + statusSpace);
    gfx_SetColor(FL_WHITE);
    gfx_Rectangle(0, 0, BORDER_SIZE, BORDER_SIZE);
    
    ti_CloseAll();
    while ((numPacks < BOARD_SIZE / PACK_SELECT_SPACING) && (var_name = ti_Detect(&search_pos, search_string)) != NULL) {
        
        packVar = ti_Open(var_name, "r");
        if (packVar != NULL) {
            ti_Seek(4, SEEK_SET, packVar);    // skip the identifier
            ti_Read(&nameLength, 1, 1, packVar);
            ti_Read(name, nameLength, 1, packVar);
            name[nameLength] = '\0';
            gfx_PrintStringXY(name, (BOARD_SIZE - gfx_GetStringWidth(name)) / 2, numPacks*PACK_SELECT_SPACING + 10);
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
                gfx_Rectangle_NoClip(5, selection*PACK_SELECT_SPACING + 5, BOARD_SIZE - 9, PACK_SELECT_SPACING);
                
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
                gfx_Rectangle_NoClip(5, selection*PACK_SELECT_SPACING + 5, BOARD_SIZE - 9, PACK_SELECT_SPACING);
            }

        } while (kb_Data[1] != kb_2nd && kb_Data[6] != kb_Enter && kb_Data[6] != kb_Clear);
    } else {
        gfx_PrintStringXY("No packs available", 10, 10);
        delay(2000);
        return NULL;
    }
    if (kb_Data[6] == kb_Clear) {
        return NULL;
    }
    
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

    level->dim = pack->levelDimensions[number];
    level->board = calloc(level->dim * level->dim, sizeof(uint8_t));
    offset = pack->levelOffset;
    for (color = 0; color < number; ++color) {
        offset += pack->levelSizes[color];
    }
    
    ti_Seek(offset, SEEK_SET, packVar);    // skip the identifier
    for (color = 1; color <= pack->levelSizes[number] / 2; ++color) {    
        ti_Read(&node, 1, 1, packVar);
        level->board[node] = color;
        ti_Read(&node, 1, 1, packVar);
        level->board[node] = color;
    }
    level->flows = pack->levelSizes[number] / 2;
    level->pipe  = level->dim * level->dim - level->flows;
    level->number = number;
    
    ti_CloseAll();
    
    return level;
}

int selectLevel(flow_pack_t *pack, uint8_t *progress, uint8_t initSelection) {
    uint8_t selection;
    uint8_t i;
    char levelText[4];
    kb_key_t key = 0;
    uint8_t star[20] = {
                            23, 2,
                            28, 18,
                            45, 18,
                            31, 28,
                            37, 44,
                            23, 34,
                            9, 44,
                            15, 28,
                            1, 18,
                            18, 18
                        };
    uint8_t check[12] = {
                            8, 18,
                            19, 29,
                            38, 6,
                            43, 10,
                            20, 39,
                            4, 22
                        };
    gfx_SetDrawBuffer();
    gfx_FillScreen(FL_BLACK);
    gfx_SetTextFGColor(FL_WHITE);
    
    gfx_SetTextScale(1, 1);
    gfx_PrintStringXY("Select", statusX, statusY);
    gfx_PrintStringXY("Level", statusX, statusY + statusSpace);
    gfx_SetTextScale(1, 2);
    gfx_PrintStringXY("FlowCE", statusX, titleY);
    //gfx_SetColor(FL_WHITE);
    
    selection = (initSelection / 25) * 25;
    while (kb_AnyKey());
    
    do {
        uint8_t keyCounter = 0;
        
        
        //kb_Scan();
        
        
        
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
        gfx_SetColor(FL_CURSOR_COLOR);

        gfx_FillRectangle_NoClip((selection % 5) * 48 + 1, ((selection / 5) % 5) * 48 + 1, 47, 47);
        if (progress[selection]) {
            gfx_SetColor(FL_GRAY);
            
        }
        gfx_SetColor(FL_WHITE);
        gfx_Rectangle_NoClip(0, 0, BORDER_SIZE, BORDER_SIZE);
        for (i = 48; i < BORDER_SIZE; i += 48) {
            gfx_VertLine_NoClip(i, 0, BOARD_SIZE);
            gfx_HorizLine_NoClip(0, i, BOARD_SIZE);
        }
        for (i = 25 * (selection / 25);
                i < ((25 * (selection / 25) + 25 < pack->numLevels) ? 25 * (selection / 25) + 25 : pack->numLevels);
                ++i) {
            uint8_t xc = (i % 5) * 48 + 1;
            uint8_t yc = ((i / 5) % 5) * 48 + 1;
            gfx_SetColor(FL_GRAY);
            if (progress[i] == 0x3) {
                polygonXY_NoClip(star, 10, xc, yc);
                gfx_FloodFill(xc + 18, yc + 34, FL_GRAY);
            } else if (progress[i] == 0x1) {
                polygonXY_NoClip(check, 6, xc, yc);
                gfx_FloodFill(xc + 18, yc + 34, FL_GRAY);
            }
            
            gfx_SetColor(FL_WHITE);
            sprintf(levelText, "%d", i + 1);
            gfx_SetTextXY(xc + 23 - (gfx_GetStringWidth(levelText) / 2), yc + 15);
            gfx_PrintString(levelText);
        }
        
        gfx_Blit(gfx_buffer);
        
        while (kb_AnyKey() && keyCounter < KEY_REPEAT_DELAY / 3) {
            ++keyCounter;
        }
        do {
            kb_Scan();
        } while (!(key = kb_Data[7]) && kb_Data[6] != kb_Enter && kb_Data[6] != kb_Clear);
        
    } while (kb_Data[1] != kb_2nd && kb_Data[6] != kb_Enter && kb_Data[6] != kb_Clear);
    if (kb_Data[6] == kb_Clear) {
        return -1;
    }
    
    gfx_SetDrawScreen();
    return selection;
}

void drawNodes(flow_level_t *level) {
    uint8_t i, dim = level->dim;
    
    for (i = 0; i < dim * dim; ++i) {
        uint8_t bz = BOARD_SIZE / (2 * dim);
        uint8_t node = level->board[i];
        
        if (node) {
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
    uint8_t boardSize, exit, selection, lastSelection;
    uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION];
    uint8_t dim = level->dim;
    kb_key_t key;
    uint16_t movesMade = 0;
    uint16_t keyCounter;
    char text[20];
    pipeComplete = 0;
    flowsComplete = 0;
    
    gfx_SetDrawBuffer();
    gfx_FillScreen(FL_BLACK);
    //gfx_SetTextFGColor(FL_WHITE);
    //gfx_SetTextScale(1, 1);
    gfx_SetColor(FL_BORDER_COLOR);
    
    for (i = 0; i <= level->dim; ++i) {
        uint8_t pos = (i * BOARD_SIZE) / level->dim;
        gfx_Line_NoClip(0, pos, BOARD_SIZE, pos);
        gfx_Line_NoClip(pos, 0, pos, BOARD_SIZE);
    }
    for (x = 0; x < dim; ++x) {
        for (y = 0; y < dim; ++y) {
            board[x][y] = level->board[x + y * dim];
            if (board[x][y]) {
                colorsComplete[board[x][y]] = 0;
                board[x][y] |= 0x0100;
            }
        }
    }
    x = y = 0;
    selection = 0;
    lastSelection = 0;
    gfx_SetTextFGColor(FL_WHITE);
    gfx_SetTextScale(1, 2);
    gfx_PrintStringXY("FlowCE", statusX, titleY);
    
    gfx_SetTextScale(1, 1);

    sprintf(text, "level %u", level->number + 1);
    gfx_PrintStringXY(text, statusX, statusY);
    sprintf(text, "%ux%u", dim, dim);
    gfx_PrintStringXY(text, statusX, statusY + statusSpace);


    drawNodes(level);
    
    while(kb_AnyKey());
    
    // game loop:
    exit = 0;
    fillCursor(x, y, level->dim, FL_CURSOR_COLOR);
    for (;;) {
        gfx_SetColor(FL_BLACK);
        gfx_FillRectangle_NoClip(statusX, statusY + 3*statusSpace, 319 - statusX, 4*statusSpace);
        
        gfx_PrintStringXY("flows", statusX, statusY + 3*statusSpace);
        sprintf(text, "%u/%u", flowsComplete, level->flows);
        gfx_PrintStringXY(text, (LCD_WIDTH - 1) - gfx_GetStringWidth(text), statusY + 3*statusSpace);
        
        gfx_PrintStringXY("moves", statusX, statusY + 4*statusSpace);
        sprintf(text, "%u", movesMade);
        gfx_PrintStringXY(text, (LCD_WIDTH - 1) - gfx_GetStringWidth(text), statusY + 4*statusSpace);
        
        gfx_PrintStringXY("pipe", statusX, statusY + 5*statusSpace);
        sprintf(text, "%u%%", (100 * pipeComplete) / level->pipe);
        gfx_PrintStringXY(text, (LCD_WIDTH - 1) - gfx_GetStringWidth(text), statusY + 5*statusSpace);
        
        gfx_Blit(gfx_buffer);
        
        keyCounter = 0;
        while (kb_AnyKey() && keyCounter < KEY_REPEAT_DELAY) {
            ++keyCounter;
        }
        
        if (exit) {
            return exit - 1;
        }
            
        do {
            kb_Scan();
        } while (
                !kb_Data[7] &&
                kb_Data[6] != kb_Clear &&
                kb_Data[6] != kb_Enter &&
                kb_Data[1] != kb_2nd
                );
               
        if (kb_Data[6] == kb_Clear) {
            // quit button pressed
            exit = 1;
            continue;
        } else if (kb_Data[6] == kb_Enter || kb_Data[1] == kb_2nd) {
            // selection button pressed
            if (selection) {
                lastSelection = selection;
                selection = 0;
                clearPathMemory();
            } else if (board[x][y]) {
                selection = board[x][y] & 0x00FF;
                if (selection != lastSelection) {
                    ++movesMade;
                }
                if (colorsComplete[selection]) {
                    colorsComplete[selection] = 0;
                    --flowsComplete;
                }
                if ((board[x][y] & 0xFF00) == 0x0100) {  // this is the first node
                    erasePipe(x, y, board, level);
                    for (x0 = 0; x0 < dim; ++x0) {
                        for (y0 = 0; y0 < dim; ++y0) {
                            if ((x != x0 || y != y0) && board[x0][y0] == (0x0100 | selection)) { // this is the other node
                                board[x0][y0] &= 0x00FF;
                                x0 = 250;
                                break;
                            }
                        }
                    }
                    
                } else if ((board[x][y] & 0xFF00) != 0x0100 && level->board[x+dim*y]) {  // the wrong node was selected
                    for (x0 = 0; x0 < dim; ++x0) {
                        for (y0 = 0; y0 < dim; ++y0) { // find the first node
                            if (board[x0][y0] == (0x0100 | selection)) {
                                erasePipe(x0, y0, board, level);
                                board[x0][y0] = selection;
                                x0 = 250;
                                break;
                            }
                        }
                    }
                    board[x][y] = 0x0100 | selection;
                } else {
                    erasePipeFrom(board[x][y], board, level);
                }
                fillCursor(x, y, level->dim, FL_BLACK);
                fillCursor(x, y, level->dim, FL_CURSOR_COLOR);
            }
            
            dbg_sprintf(dbgout, "Selection: %d\n", selection);
        } else if (kb_Data[7]) {
            // arrow key pressed
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
            dbg_sprintf(dbgout, "Moved to (%d, %d), cell value=%x\n", x, y, board[x][y]);
            if ((x0 != x || y0 != y) && selection) {
                gfx_SetColor(selection);
                if (!board[x][y]) { // nothing there yet
                    ++pipeComplete;
                    drawPipe(x0, y0, x, y, dim);
                    board[x][y] = board[x0][y0] + 0x0100;
                } else if ((board[x][y] & 0x00FF) == selection) { // this color already there
                    if ((board[x][y] & 0xFF00) == 0x00) {   // this is the end node
                        colorsComplete[selection] = 1;
                        ++flowsComplete;
                        ++pipeComplete;
                        board[x][y] = board[x0][y0] + 0x0100;
                        drawPipe(x0, y0, x, y, dim);
                        lastSelection = selection;
                        selection = 0;
                        clearPathMemory();
                    } else {
                        uint8_t xs, ys, restoreColor;
                        erasePipeFrom(board[x][y], board, level);
                        for (ys = 0; ys < dim; ++ys) {
                            for (xs = 0; xs < dim; ++xs) {
                                dbg_sprintf(dbgout, "%4x ", board[xs][ys]);
                            }
                            dbg_sprintf(dbgout, "\n");
                        }
                        for (ys = 0; ys < dim; ++ys) {
                            for (xs = 0; xs < dim; ++xs) {
                                if (!board[xs][ys] && (restoreColor = scanPathMemory(xs, ys, selection))) {  // restore pipe if any
                                        restorePipe(restoreColor, board, level);
                    
                                }
                            }
                            dbg_sprintf(dbgout, "\n");
                        }
                    }
                
                } else { // some other color already there
                    if (level->board[x+dim*y]) {
                        // colliding with node of another color
                        x = x0; // roll back
                        y = y0;
                    } else {
                        // colliding with pipe of another color
                        if (colorsComplete[0xFF & board[x][y]]) {
                            colorsComplete[0xFF & board[x][y]] = 0;
                            --flowsComplete;
                        }
                        erasePipeFrom(board[x][y] - 0x0100, board, level);
                        gfx_SetColor(selection);
                        ++pipeComplete;
                        drawPipe(x0, y0, x, y, dim);
                        board[x][y] = board[x0][y0] + 0x0100;
                    }
                        
                }
                if (flowsComplete == level->flows && pipeComplete == level->pipe) {
                    exit = 2 + (movesMade == level->flows);
                } 
            }
            fillCursor(x, y, level->dim, FL_CURSOR_COLOR);
        }
        
        
    }

    //gfx_SetColor(FL_BORDER_COLOR);

    return exit - 1;
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
            gfx_FillRectangle_NoClip(x0 - radius, y0, radius * 2 + 1, y1 - y0);
        } else {
            gfx_FillRectangle_NoClip(x0 - radius, y1, radius * 2 + 1, y0 - y1);
        }
    } else {
        if (x0 < x1) {
            gfx_FillRectangle_NoClip(x0, y0 - radius, x1 - x0, radius * 2 + 1);
        } else {
            gfx_FillRectangle_NoClip(x1, y0 - radius, x0 - x1, radius * 2 + 1);
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
    uint8_t x1 = ((x + 1) * BOARD_SIZE) / dim - 1;
    uint8_t y1 = ((y + 1) * BOARD_SIZE) / dim - 1;
    gfx_FloodFill(x0, y0, color);
    if (gfx_GetPixel(x1, y0) != color) {
        gfx_FloodFill(x1, y0, color);
    }
    if (gfx_GetPixel(x1, y1) != color) {
        gfx_FloodFill(x1, y1, color);
    }
    if (gfx_GetPixel(x0, y1) != color) {
        gfx_FloodFill(x0, y1, color);
    }
}

void erasePipeFrom(uint16_t key, uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION], flow_level_t *level) {
    uint8_t x, y;
    for (x = 0; x < level->dim; ++x) {
        for (y = 0; y < level->dim; ++y) {
            if (board[x][y] == key) {
                uint8_t bz = BOARD_SIZE / (2 * level->dim);
                
                erasePipe(x, y, board, level);
                gfx_SetColor(key & 0x00FF);
    
                x = (x * BOARD_SIZE) / level->dim + bz;
                y = (y * BOARD_SIZE) / level->dim + bz;
                //gfx_FillCircle_NoClip(x, y, BOARD_SIZE / (6 * dim));
                
                x = MAX_BOARD_DIMENSION;
                break;
            }
        }
    }
}
        
void erasePipe(uint8_t x, uint8_t y, uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION], flow_level_t *level) {
    uint8_t x1, y1, exit;
    uint8_t x0 = x;
    uint8_t y0 = y;
    uint8_t dim = level->dim;
    uint8_t color = board[x][y] & 0x00FF;
    uint16_t count = board[x][y] & 0xFF00;
    uint8_t restoreColor;
    path_point *first, *last;
    
    x1 = x;
    y1 = y;
    first = (path_point *)malloc(sizeof(path_point));
    last = first;
    first->x = x;
    first->y = y;
    for (exit = 1; exit;) {
        uint16_t key, end;
        count += 0x0100;
        key =  count | color;
        end = 0xFF00 | color;
   
        if        (x+1 < dim && board[x+1][y] == key) {
            ++x1;
        } else if (y+1 < dim && board[x][y+1] == key) {
            ++y1;
        } else if (x-1 >= 0  && board[x-1][y] == key) {
            --x1;
        } else if (y-1 >= 0  && board[x][y-1] == key) {
            --y1;
        }
        if (x1 != x || y1 != y) {
            path_point *newPoint = (path_point *)malloc(sizeof(path_point));
            uint8_t xMax = max(x, x1);
            uint8_t yMax = max(y, y1);
            gfx_SetColor(FL_BLACK);
            drawPipe(x, y, x1, y1, dim);    // erase the pipe
            
            newPoint->x = x1;
            newPoint->y = y1;
            last->next = newPoint;
            last = newPoint;
            --pipeComplete;
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
            if (!level->board[x1+dim*y1]) {
                board[x1][y1] = 0;  // remove from the board
            } else {
                board[x1][y1] &= 0x00FF;
            }
            x = x1;
            y = y1;
        } else {
            if ((board[x0][y0] & 0xFF00) > 0x0100) {    // fix the pipe drawing
                x = x0;
                y = y0;
                key = board[x0][y0] - 0x0100;
                if        (x+1 < dim && board[x+1][y] == key) {
                    ++x;
                } else if (y+1 < dim && board[x][y+1] == key) {
                    ++y;
                } else if (x-1 >= 0  && board[x-1][y] == key) {
                    --x;
                } else if (y-1 >= 0  && board[x][y-1] == key) {
                    --y;
                }
                gfx_SetColor(color);
                drawPipe(x, y, x0, y0, dim);
            }
            exit = 0;
            if (pathMemory[color] && pathMemory[color]->x == last->x && pathMemory[color]->y == last->y) {
                last->next = pathMemory[color]->next;
                free(pathMemory[color]);
            } else {
                last->next = pathMemory[color];
            }
            pathMemory[color] = first;
        } 
    }
    drawNodes(level);
}

uint8_t endMenu(char *title, char **options, uint8_t num, uint8_t mask) {
    const uint8_t padding = 10;
    const uint8_t boxHeight = 20;
    const uint8_t width = 180;
    uint8_t height;
    uint8_t x = (BOARD_SIZE - width) / 2;
    uint8_t y;
    uint8_t selection = 0;
    uint8_t i;
    uint8_t numOptions = num;
    dbg_sprintf(dbgout, "mask=%x\n", mask);
    for (i = 0; i < numOptions; ++i) {
        if (!(mask & (0x1 << i))) {
            --num;
        }
    }
    height = padding * (2 + num) + boxHeight * (1 + num);
    y = (BOARD_SIZE - height) / 2;
    
    gfx_SetDrawBuffer();
    gfx_SetTextFGColor(FL_WHITE);

    gfx_SetColor(FL_BLACK);
    gfx_FillRectangle_NoClip(x, y, width, height); 
    gfx_SetColor(FL_BORDER_COLOR);
    gfx_Rectangle_NoClip(x, y, width, height);
    gfx_SetTextScale(1,1);
    gfx_PrintStringXY(
                title,
                x + padding + (width - 2 * padding - gfx_GetStringWidth(title)) / 2,
                y + padding + 6
            );
    
    do {
        uint8_t skipped = 0;
        

        for (i = 0; i < num; ++i) {
            
            while (!(mask & (0x1 << (i + skipped)))) {
                ++skipped;
            }
            dbg_sprintf(dbgout, "i=%u skipped=%u\n", i, skipped);
            gfx_SetColor((i == selection) ? FL_CURSOR_COLOR : FL_BLACK);
            gfx_FillRectangle_NoClip(x + padding, y + padding + (padding + boxHeight) * (i + 1), width - 2 * padding, boxHeight);
            gfx_SetColor(FL_BORDER_COLOR);
            gfx_Rectangle_NoClip(x + padding, y + padding + (padding + boxHeight) * (i + 1), width - 2 * padding, boxHeight);
            gfx_PrintStringXY(
                options[i + skipped],
                x + padding + (width - 2 * padding - gfx_GetStringWidth(options[i + skipped])) / 2,
                y + padding + (padding + boxHeight) * (i + 1) + 6
            );
            
        }
        gfx_Blit(gfx_buffer);
        while (kb_AnyKey()) ;
        
        do {
            kb_Scan();
        } while (!kb_Data[1] && !kb_Data[6] && !kb_Data[7]);
        
        switch (kb_Data[7]) {
            case kb_Down:
                selection = (selection + 1) % num;
                break;
            case kb_Up:
                selection = (selection) ? selection - 1 : num - 1;
                break;
            default:
                break;
                        
        }
        
    } while (kb_Data[6] != kb_Enter && kb_Data[1] != kb_2nd);
    
    for (i = 0; i <= selection; ++i) {
        if (!(mask & (0x1 << i))) {
            ++selection;
        }
    }
    
    return selection;
}

void saveProgress(flow_pack_t *pack, uint8_t *progress) {
    ti_var_t saveData;
    char *varName = "FLOWDATA";
    uint16_t bits = 2 * pack->numLevels;
    uint8_t length = 1 + ((bits - 1) / 8);
    uint8_t *buffer = calloc(length, sizeof(uint8_t));
    uint16_t i;
    uint8_t offset;
    char packVarName[9];
    size_t chunks;
    char *ptr = (char*)os_GetSystemStats();
    
    saveData = ti_Open(varName, "r+");
    if (!saveData) {
        saveData = ti_Open(varName, "a+");
        dbg_sprintf(dbgout, "Opened for appending");
    }
    
    for (i = 0; i < bits; i += 2) {
        buffer[i / 8] |= (progress[i / 2] << (i % 8));
    }
    
    dbg_sprintf(dbgout, "Saving data for %s", pack->appvarName);
    //if (memcmp(zeroID, &ptr[28], 10)) {
        ti_Write(&ptr[28], 1, 10, saveData); // write the ID
    //}
    offset = 0;
    for (;;) {
        chunks = ti_Read(packVarName, 1, 9, saveData);
        if (ti_Tell(saveData) >= ti_GetSize(saveData)) {
            // append
            ti_Seek(-9, SEEK_CUR, saveData);
            dbg_sprintf(dbgout, "save append\n");
            ti_Write(pack->appvarName, 1, 9, saveData);
            ti_Write(&length, 1, 1, saveData);
            break;
        } else if (strcmp(pack->appvarName, packVarName)) {
            // seek to next
            dbg_sprintf(dbgout, "save seek\n");
            ti_Read(&offset, 1, 1, saveData);
            ti_Seek(offset, SEEK_CUR, saveData);
        } else {
            // found
            dbg_sprintf(dbgout, "save found\n");
            ti_Read(&offset, 1, 1, saveData);
            break;
        }
        
    }
    ti_Write(buffer, 1, length, saveData);
    ti_SetArchiveStatus(true, saveData);
    ti_Close(saveData);
    free(buffer);
}

uint8_t *loadProgress(flow_pack_t *pack) {
    ti_var_t saveData;
    uint8_t i;
    uint8_t *progress = calloc(pack->numLevels, sizeof(uint8_t));
    char *varName = "FLOWDATA";
    char packVarName[9];
    uint16_t bits = 2 * pack->numLevels;
    uint8_t length = 1 + ((bits - 1) / 8);
    uint8_t *buffer;
    uint8_t offset;
    
    saveData = ti_Open(varName, "r");
    if (!saveData) {    // does not already exist
        return progress;
    } else {
        char *saveID = (char*)ti_GetDataPtr(saveData);
        char *ptr = (char*)os_GetSystemStats();
        //ti_Read(&saveID, 1, 10, saveData);
        if (memcmp(&ptr[28], saveID, 10) && memcmp(zeroID, saveID, 10) && memcmp(&ptr[28], zeroID, 10)) {
            ti_Delete(varName);
            return progress;
        }
    }
    offset = 10;
    do {
        ti_Seek(offset, SEEK_CUR, saveData);
        ti_Read(packVarName, 1, 9, saveData);
        ti_Read(&offset, 1, 1, saveData);
    } while (ti_Tell(saveData) < ti_GetSize(saveData) && strcmp(packVarName, pack->appvarName));
    
    if (ti_Tell(saveData) >= ti_GetSize(saveData)) {
        return progress;
    }
    
    buffer = malloc(length);
    ti_Read(buffer, 1, length, saveData);
    for (i = 0; i < bits; i += 2) {
        progress[i / 2] = (buffer[i / 8] & (0x3 << (i % 8))) >> (i % 8);
    }
    free(buffer);
    ti_Close(saveData);
    return progress;
}

void polygonXY_NoClip(uint8_t *points, unsigned num_points, uint24_t x, uint8_t y) {
    unsigned i;
    
    for (i = 2; i < num_points * 2; i += 2) {
        gfx_Line_NoClip(x + points[i - 2], y + points[i - 1], x + points[i], y + points[i + 1]);
    }
    gfx_Line_NoClip(x + points[0], y + points[1], x + points[i - 2], y + points[i - 1]);
}

void clearPathMemory() {
    uint8_t i;
    for (i = 0; i < sizeof(pathMemory) / sizeof(path_point *); ++i) {
        clearLinkedList(pathMemory[i]);
        pathMemory[i] = NULL;
    }
}

void clearLinkedList(path_point *head) {
    path_point *next;
    while (head != NULL) {
        next = head->next;
        free(head);
        head = next;
    }
}

uint8_t scanPathMemory(uint8_t x, uint8_t y, uint8_t ignoreColor) {
    uint8_t i;
    for (i = 1; i < sizeof(pathMemory) / sizeof(path_point *); ++i) {
        if (i != ignoreColor &&
            pathMemory[i] &&
            pathMemory[i]->next &&
            pathMemory[i]->next->x == x &&
            pathMemory[i]->next->y == y
            ) {
                //dbg_sprintf(dbgout, "restore point found at (%u, %u), color=%u\n", pathMemory[i]->x, pathMemory[i]->y, i);
            return i;
        }
    }
    return 0;
}

void restorePipe(uint8_t color, uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION], flow_level_t *level) {
    path_point *head = pathMemory[color];
    path_point *next = (head != NULL) ? head->next : NULL;
    //dbg_sprintf(dbgout, "restorePipe()\n");
    gfx_SetColor(color);

    if (head && level->board[head->x + level->dim * head->y]) {
        ++pipeComplete;
    }
    while (next != NULL && ((board[next->x][next->y] == 0) || ((board[next->x][next->y] & 0xFF) == color))) {
        //dbg_sprintf(dbgout, "redraw");
        drawPipe(head->x, head->y, next->x, next->y, level->dim);
        board[next->x][next->y] = board[head->x][head->y] + 0x0100;
        if ((board[head->x][head->y] & 0xFF00) != 0x0100) {
            ++pipeComplete;
            if (level->board[next->x + level->dim * next->y]) {
                colorsComplete[color] = 1;
                ++flowsComplete;
            }
        }
        free(head);
        head = next;
        next = head->next;        
    }
    if (head == NULL) {
        pathMemory[color] = NULL;
    } else {
        pathMemory[color] = head;
    }
}