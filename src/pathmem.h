#ifndef PATHMEM_H
#define PATHMEM_H

void clearPathMemory();
void clearLinkedList(path_point *head);
uint8_t scanPathMemory(uint8_t x, uint8_t y, uint8_t ignoreColor);
void restorePipe(uint8_t color, uint16_t board[MAX_BOARD_DIMENSION][MAX_BOARD_DIMENSION], flow_level_t *level);

#endif