////////////////////////////////////////
// { FlowCE } { 0.1 }
// Author: Jonathan Bush
// License: GPL
// Description: Flow Free for the CE
////////////////////////////////////////



#include "main.h"
#include "gameplay.h"
#include "pack.h"
#include "progress.h"
#include "pathmem.h"
#include "level.h"

void main(void) {
    int levelNum;
    flow_pack_t *selected;
    flow_level_t *level;
    uint8_t *progress;
    
    gfx_Begin();
    
    // Use the custom colors
    gfx_SetPalette(flow_gfx_pal, sizeof(flow_gfx_pal), 0);

    //srand(rtc_Time());
    displayTitleScreen();
start:

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
        
        while (levelNum >= 0) {
            
            level = loadLevel(selected, levelNum);

            switch (playLevel(level, progress[levelNum])) {
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
                case 4:
                    levelNum -= (levelNum > 0);
                    goto skip;
                    break;
                case 5:
                    levelNum += (levelNum + 1 < selected->numLevels);
                    goto skip;
                    break;
            }
            switch (selection) {
                case 0: // Next level
                    ++levelNum;
                    break;
                case 1: // Try Again
                    // play the level again
                    break;
                case 2: // Select Level
                    levelNum = selectLevel(selected, progress, levelNum);
                    break;
                default: // Quit
                    levelNum = -2;
                    break;
            }
skip:

            free(level->board);
            free(level);
            clearPathMemory();
        }
        
        saveProgress(selected, progress);
        free(progress);
        
        if (levelNum == -1) {
            free(selected->levelDimensions);
            free(selected->levelSizes);
            free(selected);
            while (kb_AnyKey()) ;
            goto start;
        }
        
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
        
    } while (kb_Data[6] != kb_Enter && kb_Data[1] != kb_2nd && kb_Data[6] != kb_Clear);
    
    if (kb_Data[6] == kb_Clear) {
        return 255;
    }
    
    for (i = 0; i <= selection; ++i) {
        if (!(mask & (0x1 << i))) {
            ++selection;
        }
    }
    
    return selection;
}

void polygonXY_NoClip(uint8_t *points, unsigned num_points, uint24_t x, uint8_t y) {
    unsigned i;
    
    for (i = 2; i < num_points * 2; i += 2) {
        gfx_Line_NoClip(x + points[i - 2], y + points[i - 1], x + points[i], y + points[i + 1]);
    }
    gfx_Line_NoClip(x + points[0], y + points[1], x + points[i - 2], y + points[i - 1]);
}