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

#define TITLE_SCREEN_DELAY 1400


typedef struct flow_pack_t {
	char name[20];
	char appvarName[9];
	uint8_t numLevels;
	uint8_t *levelDimensions;	// dimensions of level board
	uint8_t *levelSizes;		// lengths of level data in bytes
} flow_pack_t;

	
/* function prototypes */
void displayTitleScreen(void);
flow_pack_t * loadPack(char *appvarName, int8_t complete);
flow_pack_t * selectLevelPack();

flow_pack_t *packs_available[20];

void main(void) {
    uint8_t i, j, level;
	flow_pack_t *selected;
    /* This function cleans up the screen and gets everything ready for the OS */
    pgrm_CleanUp();
    gfx_Begin( gfx_8bpp );
    //gfx_SetMonospaceFont(false);
    gfx_SetPalette(flow_gfx_pal, 288, 0);   // Use the custom colors
    // Do all the stuff here
    //srand(rtc_Time());
    displayTitleScreen();
	selected = selectLevelPack();
	dbg_sprintf(dbgout, "Selected pack: %s", selected->name);
	//displayTitleScreen();
	delay(1500);
    
    //while(!kb_AnyKey());
	
	for (i = 0; i < sizeof(packs_available)/sizeof(flow_pack_t *); ++i) {
		if (packs_available[i] != NULL) {
			free(packs_available[i]->levelDimensions);
			free(packs_available[i]->levelSizes);
			free(packs_available[i]);
		}
	}
	
    gfx_End();
    //pgrm_CleanUp();
}

void displayTitleScreen() {
    uint8_t xPos, yPos, i;
    const uint8_t colors[10] = {FL_RED,FL_DARK_GREEN,FL_BLUE,FL_YELLOW,FL_WHITE,FL_WHITE};
    const char title[] = "FlowCE";
    char single[];

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


flow_pack_t * loadPack(char *appvarName, int8_t complete) {
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
	
	ti_Seek(4, SEEK_SET, packVar);	// skip the identifier
	ti_Read(&nameLength, 1, 1, packVar);
	ti_Read(pack->name, nameLength, 1, packVar);
	pack->name[nameLength] = '\0';
	ti_Read(&(pack->numLevels), 1, 1, packVar);
	
	dbg_sprintf(dbgout, "Pack name length: %d\n", nameLength);
	dbg_sprintf(dbgout, "Pack name: %s\n", pack->name);
	
	if (complete) {
		pack->levelSizes = malloc((size_t)pack->numLevels);
		pack->levelDimensions = malloc((size_t)pack->numLevels);

		for (i = 0; i < pack->numLevels; ++i) {
			ti_Read(&(pack->levelSizes[i]), 1, 1, packVar);
			ti_Read(&(pack->levelDimensions[i]), 1, 1, packVar);
		}
	}
	
	ti_CloseAll();
	
	return pack;
}



flow_pack_t * selectLevelPack() {
	uint8_t *search_pos = NULL;
	const char search_string[] = "FLCE";
	uint8_t selection = 0;
	uint8_t numPacks = 0;
	kb_key_t key = 0xF0;
	//uint8_t nameLength;
	char *var_name;
	
	gfx_FillScreen(FL_BLACK);
	gfx_SetTextFGColor(FL_WHITE);
	gfx_SetTextScale(1, 1);
	
	while ((var_name = ti_Detect(&search_pos, search_string)) != NULL && numPacks < 20) {
		dbg_sprintf(dbgout, "here2");
		packs_available[numPacks] = loadPack(var_name, 0);
		if (packs_available[numPacks] != NULL) {
			//dbg_sprintf(dbgout, "Pack name: %s\n", packs_available[numPacks]->name);
			gfx_PrintStringXY(packs_available[numPacks]->name, 10, numPacks*6 + 10);
			++numPacks;
		}
	}
	
	do {
		kb_Scan();
		
		if (key != kb_Data[7]) {
			key = kb_Data[7];
			
			gfx_SetColor(FL_BLACK);
			gfx_Rectangle_NoClip(5, selection*6 + 5, 80, 20);
			
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
			gfx_Rectangle_NoClip(5, selection*6 + 5, 80, 20);
			
		}
		
	} while (kb_Data[1] != kb_2nd && kb_Data[6] != kb_Enter);
	
	free(search_pos);
	free(var_name);
	return packs_available[selection];
}