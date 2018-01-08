#ifndef LEVEL_H
#define LEVEL_H

flow_level_t * loadLevel(flow_pack_t *pack, uint8_t number);
int selectLevel(flow_pack_t *pack, uint8_t *progress, uint8_t initSelection);

#endif