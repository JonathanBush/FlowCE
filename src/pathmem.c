#include "main.h"
#include "gameplay.h"

path_point *pathMemory[24];

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