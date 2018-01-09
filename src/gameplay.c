#include "main.h"
#include "gameplay.h"
#include "pathmem.h"

uint16_t pipeComplete;
uint8_t flowsComplete;
uint8_t colorsComplete[20];

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
    gfx_SetColor(FL_BORDER_COLOR);
    
    for (i = 0; i <= level->dim; ++i) {
        uint8_t pos = (i * BOARD_SIZE) / level->dim;
        gfx_HorizLine_NoClip(0, pos, BORDER_SIZE - 1);
        gfx_VertLine_NoClip(pos, 0, BOARD_SIZE);
        //gfx_Line_NoClip(0, pos, BOARD_SIZE, pos);
        //gfx_Line_NoClip(pos, 0, pos, BOARD_SIZE);
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
        
        fillCursor(x, y, level->dim, FL_CURSOR_COLOR);
        
        gfx_Blit(gfx_buffer);
        
        fillCursor(x, y, level->dim, FL_BLACK);
        
        keyCounter = 0;
        do {
            kb_Scan();
            ++keyCounter;
        } while ((kb_Data[7] && keyCounter < KEY_REPEAT_DELAY) || kb_Data[1] || kb_Data[6]);
        
        if (exit) {
            break;
        }
            
        do {
            kb_Scan();
        } while (//!kb_AnyKey());
                !kb_Data[7] &&
                !kb_Data[6] &&
                !kb_Data[1]
                );
               
        if (kb_Data[6] == kb_Clear) {
            // quit button pressed
            exit = 1;
        } else if (kb_Data[6] == kb_Enter || kb_Data[1] == kb_2nd) {
            // selection button pressed
            if (selection) {
                lastSelection = selection;
                selection = 0;  // deselect
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
                                //board[x0][y0] &= 0x00FF;
                                board[x0][y0] = selection;
                                x0 = MAX_BOARD_DIMENSION;
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
                                x0 = MAX_BOARD_DIMENSION;
                                break;
                            }
                        }
                    }
                    board[x][y] = 0x0100 | selection;
                } else {
                    erasePipeFrom(board[x][y], board, level);
                }
            }
            
            dbg_sprintf(dbgout, "Selection: %d\n", selection);
        } else if (kb_Data[7]) {
            // arrow key pressed
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
                    } else {    // this is the pipe
                        uint8_t xs, ys, restoreColor;
                        erasePipeFrom(board[x][y], board, level);
                        for (ys = 0; ys < dim; ++ys) {
                            for (xs = 0; xs < dim; ++xs) {
                                dbg_sprintf(dbgout, "%4x ", board[xs][ys]);
                            }
                            dbg_sprintf(dbgout, "\n");
                        }
                        for (ys = 0; ys < dim; ++ys) {  // scan for other pipes to restore
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
            //fillCursor(x, y, level->dim, FL_CURSOR_COLOR);
        } else if (kb_Data[1] == kb_Yequ) {
            return 4;   // browse back
        } else if (kb_Data[1] == kb_Graph) {
            return 5;   // browse forward
        }
        
        
    }

    return exit - 1;
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

void fillCursor(uint8_t x, uint8_t y, uint8_t dim, uint8_t color) {
    uint8_t x0 = (x * BOARD_SIZE) / dim + 1;
    uint8_t y0 = (y * BOARD_SIZE) / dim + 1;
    uint8_t x1 = ((x + 1) * BOARD_SIZE) / dim - 1;
    uint8_t y1 = ((y + 1) * BOARD_SIZE) / dim - 1;
    gfx_FloodFill(x0, y0, color);
    if (gfx_GetPixel(x1, y0) != color) {    // make sure all the regions are filled
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
                xMax = (xMax * BOARD_SIZE) / dim;
                gfx_VertLine_NoClip(xMax, yTop, BOARD_SIZE / dim);
            } else { // movement in the y direction
                uint8_t xLeft = ((xMax) * BOARD_SIZE) / dim;
                yMax = (yMax * BOARD_SIZE) / dim;
                gfx_HorizLine_NoClip(xLeft, yMax, BOARD_SIZE / dim);
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