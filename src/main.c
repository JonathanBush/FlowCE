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
//#include <lib/ce/keypadc.h>
//#include <lib/ce/fileioc.h>

#include "flow_colors.h"

#define TITLE_SCREEN_DELAY 1400

/* Put function prototypes here */
void displayTitleScreen(void);


/* Initialize a string variable */

/* Put all your code here */
void main(void) {
    uint8_t i, j, level;
    /* This function cleans up the screen and gets everything ready for the OS */
    pgrm_CleanUp();
    gfx_Begin( gfx_8bpp );
    //gfx_SetMonospaceFont(false);
    gfx_SetPalette(flow_gfx_pal, 288, 0);   // Use the custom colors
    // Do all the stuff here
    //srand(rtc_Time());
    displayTitleScreen();



    /* Wait for a keypress */
    //while(!os_GetCSC());
    gfx_End();
    pgrm_CleanUp();
}

void displayTitleScreen() {
    uint8_t xPos, yPos, i;
    const uint8_t colors[10] = {FL_RED,FL_DARK_GREEN,FL_BLUE,FL_YELLOW,FL_WHITE,FL_WHITE};
    const char title[] = "FlowCE";
    char single[];

    gfx_FillScreen(16);
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