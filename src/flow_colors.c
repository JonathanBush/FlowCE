#include <graphx.h>
#include <stdint.h>

uint16_t flow_gfx_pal[18] = {
    gfx_RGBTo1555(1,4,0),       // 0 Black
    gfx_RGBTo1555(255,0,0),     // 1 Red
    gfx_RGBTo1555(7,128,2),     // 2 Green
    gfx_RGBTo1555(22,0,255),    // 3 Blue
    gfx_RGBTo1555(236,239,0),   // 4 Yellow
    gfx_RGBTo1555(253,127,0),   // 5 Orange
    gfx_RGBTo1555(0,255,255),   // 6 Light blue
    gfx_RGBTo1555(253,5,255),   // 7 Light purple
    gfx_RGBTo1555(167,39,43),   // 8 Dark red
    gfx_RGBTo1555(129,0,129),   // 9 Dark purple
    gfx_RGBTo1555(253,255,252), // 10 White
    gfx_RGBTo1555(165,167,164), // 11 Gray
    gfx_RGBTo1555(12,255,0),    // 12 Light green
    gfx_RGBTo1555(187,183,106), // 13 Greenish gray
    gfx_RGBTo1555(3,0,140),     // 14 Navy blue
    gfx_RGBTo1555(0,129,128),   // 15 Turquoise
    gfx_RGBTo1555(255,1,146),   // 16 Pink  
    gfx_RGBTo1555(125,95,62)    // 17 Border brown
};