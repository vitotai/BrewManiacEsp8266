#pragma once

#include <Arduino.h>

typedef struct _BackupStatusInfo{
    uint32_t backupflags;
    uint32_t time_stamp;
    uint32_t setting_size;
    uint32_t networkconfig_size;
    uint32_t preference_size;
} BackupStatusInfo;

class BackupServiceClass{
 public:
    BackupServiceClass();
    
    bool status(BackupStatusInfo &info);

    void backup(void);
    void restore(void);
private:
    uint32_t _sector;
    void _readFlash(uint8_t* buff,uint32_t offset, uint32_t size);
    bool _writeFlash(uint8_t* buff,uint32_t offset, uint32_t size);
};

extern BackupServiceClass BackupService;