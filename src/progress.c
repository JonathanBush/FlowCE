#include "main.h"
#include "progress.h"

const char zeroID[8] = {0,0,0,0,0,0,0,0};

void saveProgress(flow_pack_t *pack, uint8_t *progress) {
    ti_var_t saveData;
    char *varName = "FLOWDATA";  // name of the progress data AppVar
    uint16_t bits = 2 * pack->numLevels;
    uint8_t length = 1 + ((bits - 1) / 8);
    uint8_t *buffer = calloc(length, sizeof(uint8_t));  // save data to be written for this pack
    uint16_t i;
    uint8_t offset;
    char packVarName[9];
    //size_t chunks;
    const system_info_t *sys_info = os_GetSystemInfo();
    
    saveData = ti_Open(varName, "r+");
    if (!saveData) {
        saveData = ti_Open(varName, "a+");
        dbg_sprintf(dbgout, "Opened for appending");
    }

    // the completion status of each level is represented by two bits
    for (i = 0; i < bits; i += 2) {
        buffer[i / 8] |= (progress[i / 2] << (i % 8));
    }
    
    dbg_sprintf(dbgout, "Saving data for %s\n", pack->appvarName);
    ti_Write(&sys_info->calcid, 1, 8, saveData); // write the ID
    ti_Write(zeroID, 1, 2, saveData);  // maintain compatability with previous version

    offset = 0;
    for (;;) {
        if (ti_Tell(saveData) >= ti_GetSize(saveData)) {  // reached end of AppVar
            // append
            dbg_sprintf(dbgout, "save append\n");
            ti_Write(pack->appvarName, 1, 9, saveData);
            ti_Write(&length, 1, 1, saveData);  // length of progress buffer
            break;
        }
        ti_Read(packVarName, 1, 9, saveData);
        if (strcmp(pack->appvarName, packVarName)) {  // not equal
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
        const system_info_t *sys_info = os_GetSystemInfo();
        if (memcmp(&sys_info->calcid, saveID, 8) && memcmp(zeroID, saveID, 8) && memcmp(&sys_info->calcid, zeroID, 8)) {
            ti_Delete(varName);
            dbg_sprintf(dbgout, "Deleted FLOWDATA\n");
            return progress;
        }
    }
    offset = 10;  // skip initial 10 bytes - calcid and 0x0000
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