#ifndef GAMEPLAY_H
#define GAMEPLAY_H

uint8_t playLevel(flow_level_t *level, uint8_t status);
void drawNodes(flow_level_t *level);
void drawPipe(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t radius);
//void drawCursor(uint8_t x, uint8_t y, uint8_t dim);
void fillCursor(uint8_t x, uint8_t y, uint8_t dim, uint8_t color);
void erasePipe(uint8_t x0, uint8_t y0, uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION], flow_level_t *level);
void erasePipeFrom(uint16_t key, uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION], flow_level_t *level);

#endif