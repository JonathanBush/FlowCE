#include "main.h"
#include "progress.h"

const char zeroID[10] = {0,0,0,0,0,0,0,0,0,0};

void saveProgress(flow_pack_t *pack, uint8_t *progress) {
    ti_var_t saveData;
    char *varName = "FLOWDATA";
    uint16_t bits = 2 * pack->numLevels;
    uint8_t length = 1 + ((bits - 1) / 8);
    uint8_t *buffer = calloc(length, sizeof(uint8_t));
    uint16_t i;
    uint8_t offset;
    char packVarName[9];
    size_t chunks;
    char *ptr = (char*)os_GetSystemStats();
    
    saveData = ti_Open(varName, "r+");
    if (!saveData) {
        saveData = ti_Open(varName, "a+");
        dbg_sprintf(dbgout, "Opened for appending");
    }
    
    for (i = 0; i < bits; i += 2) {
        buffer[i / 8] |= (progress[i / 2] << (i % 8));
    }
    
    dbg_sprintf(dbgout, "Saving data for %s", pack->appvarName);
    //if (memcmp(zeroID, &ptr[28], 10)) {
        ti_Write(&ptr[28], 1, 10, saveData); // write the ID
    //}
    offset = 0;
    for (;;) {
        chunks = ti_Read(packVarName, 1, 9, saveData);
        if (ti_Tell(saveData) >= ti_GetSize(saveData)) {
            // append
            ti_Seek(-9, SEEK_CUR, saveData);
            dbg_sprintf(dbgout, "save append\n");
            ti_Write(pack->appvarName, 1, 9, saveData);
            ti_Write(&length, 1, 1, saveData);
            break;
        } else if (strcmp(pack->appvarName, packVarName)) {
            // seek to next
            dbg_sprintf(dbgout, "save seek\n");
            ti_Read(&offset, 1, 1, saveData);
            ti_Seek(offset, SEEK_CUR, saveData);
        } else {
            // found
            dbg_sprintf(dbgout, "save found\n");
            ti_Read(&offset, 1, 1, saveData);
            break;
        }
        
    }
    ti_Write(buffer, 1, length, saveData);
    ti_SetArchiveStatus(true, saveData);
    ti_Close(saveData);
    free(buffer);
}

uint8_t *loadProgress(flow_pack_t *pack) {
    ti_var_t saveData;
    uint8_t i;
    uint8_t *progress = calloc(pack->numLevels, sizeof(uint8_t));
    char *varName = "FLOWDATA";
    char packVarName[9];
    uint16_t bits = 2 * pack->numLevels;
    uint8_t length = 1 + ((bits - 1) / 8);
    uint8_t *buffer;
    uint8_t offset;
    
    saveData = ti_Open(varName, "r");
    if (!saveData) {    // does not already exist
        return progress;
    } else {
        char *saveID = (char*)ti_GetDataPtr(saveData);
        char *ptr = (char*)os_GetSystemStats();
        //ti_Read(&saveID, 1, 10, saveData);
        if (memcmp(&ptr[28], saveID, 10) && memcmp(zeroID, saveID, 10) && memcmp(&ptr[28], zeroID, 10)) {
            ti_Delete(varName);
            return progress;
        }
    }
    offset = 10;
    do {
        ti_Seek(offset, SEEK_CUR, saveData);
        ti_Read(packVarName, 1, 9, saveData);
        ti_Read(&offset, 1, 1, saveData);
    } while (ti_Tell(saveData) < ti_GetSize(saveData) && strcmp(packVarName, pack->appvarName));
    
    if (ti_Tell(saveData) >= ti_GetSize(saveData)) {
        return progress;
    }
    
    buffer = malloc(length);
    ti_Read(buffer, 1, length, saveData);
    for (i = 0; i < bits; i += 2) {
        progress[i / 2] = (buffer[i / 8] & (0x3 << (i % 8))) >> (i % 8);
    }
    free(buffer);
    ti_Close(saveData);
    return progress;
}