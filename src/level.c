#include "main.h"
#include "level.h"

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
    uint8_t selection, lowerBound;
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
            gfx_VertLine_NoClip(i, 0, BORDER_SIZE);
            gfx_HorizLine_NoClip(0, i, BORDER_SIZE);
        }
        
        lowerBound = 25 * (selection / 25);
        for (i = lowerBound;
                i < ((lowerBound + 25 < pack->numLevels) ? lowerBound + 25 : pack->numLevels);
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
        
        while (kb_AnyKey() && keyCounter < KEY_REPEAT_DELAY / 2) {
            ++keyCounter;
        }
        do {
            kb_Scan();
        } while (!(key = kb_Data[7]) && kb_Data[1] != kb_2nd && kb_Data[6] != kb_Enter && kb_Data[6] != kb_Clear);
        
    } while (kb_Data[1] != kb_2nd && kb_Data[6] != kb_Enter && kb_Data[6] != kb_Clear);
    if (kb_Data[6] == kb_Clear) {
        return -1;
    }
    
    gfx_SetDrawScreen();
    return selection;
}