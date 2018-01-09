#include "main.h"

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
    uint8_t nameLength,i;
    char *var_name;
    char name[20];
    ti_var_t packVar;
    char *keys[7] = {
                    "move:",
                    "{arrows}",
                    "select:",
                    "[2nd]",
                    "[enter]",
                    "back:",
                    "[clear]"
    };
    
    gfx_SetDrawScreen();
    gfx_FillScreen(FL_BLACK);
    
    gfx_SetColor(FL_WHITE);
    gfx_Rectangle(0, 0, BORDER_SIZE, BORDER_SIZE);
    gfx_SetTextScale(1, 2);
    gfx_PrintStringXY("FlowCE", statusX, titleY);
    gfx_SetTextScale(1, 1);
    gfx_PrintStringXY("Select", statusX, statusY);
    gfx_PrintStringXY("Pack", statusX, statusY + statusSpace);
    
    gfx_SetTextFGColor(FL_GRAY);
    for (i = 0; i < 7; ++i) {
        gfx_PrintStringXY(keys[i], statusX, statusY + (4 + i) * statusSpace);
    }
    gfx_SetTextFGColor(FL_WHITE);
    
    
    
    
    
    
    
    
    
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