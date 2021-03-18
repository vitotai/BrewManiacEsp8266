#include "FS.h"
#include "config.h"
#include "BackupService.h"
#if !ESP32
extern "C" {
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "spi_flash.h"
}
#include "FsEeprom.h"
#include "TimeKeeper.h"

extern FS& FileSystem;

#if SerialDebug == true
#define DBG_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#endif


extern "C" uint32_t _SPIFFS_end;

#define GuardPattern 0xF1F2F3F4L

#define Round2Next4(a) (((a) + 3) & ~0x03)

//TODO: those are defined in BrewManiacEx.cpp, should've move to commmon header
#define CONFIG_FILENAME 	"/network.cfg"
#define RECIPE_PREFERNECE 	"/userpref.cfg"

BackupServiceClass BackupService;

typedef struct _BackupHeader{
    uint32_t pattern;
    uint32_t backupflags;
    uint32_t time_stamp;
    uint32_t setting_size;
    uint32_t networkconfig_size;
    uint32_t preference_size;
    uint32_t _padding[1];
} BackupHeader;


BackupServiceClass::BackupServiceClass(void):_sector((((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE)){}

void BackupServiceClass::_readFlash(uint8_t* buff,uint32_t offset,uint32_t size){
    noInterrupts();
    spi_flash_read(_sector * SPI_FLASH_SEC_SIZE + offset, reinterpret_cast<uint32_t*>(buff), size);
    interrupts();
}

bool BackupServiceClass::_writeFlash(uint8_t* buff,uint32_t offset,uint32_t size){
    noInterrupts();
    bool ret=(spi_flash_write(_sector * SPI_FLASH_SEC_SIZE + offset, reinterpret_cast<uint32_t*>(buff), size) == SPI_FLASH_RESULT_OK);
    interrupts();
    return ret;
}

bool BackupServiceClass::status(BackupStatusInfo &info){

    BackupHeader header;
    _readFlash((uint8_t*) &header,0,sizeof(header));
    if(header.pattern !=GuardPattern) return false;
    if(header.setting_size != (uint32_t)FsEEPROM.size()) return false;
    info.preference_size = header.backupflags;
    info.time_stamp = header.time_stamp;
    info.setting_size = header.setting_size;
    info.networkconfig_size = header.networkconfig_size;
    info.preference_size = header.preference_size;
    return true;
}

void BackupServiceClass::backup(){

    BackupHeader header;
    header.pattern = GuardPattern;
    header.time_stamp = TimeKeeper.getTimeSeconds();
    header.setting_size = FsEEPROM.size();

	if(FileSystem.exists(CONFIG_FILENAME)){
        File f=FileSystem.open(CONFIG_FILENAME,"r+");
        header.networkconfig_size = f.size();
        f.close();
    }else{
        header.networkconfig_size=0;
    }

	if(FileSystem.exists(RECIPE_PREFERNECE)){
        File f=FileSystem.open(RECIPE_PREFERNECE,"r+");
        header.preference_size = f.size();
        f.close();
    }else{
        header.preference_size=0;
    }
    uint32_t bufferSize = (header.preference_size > header.networkconfig_size)? header.preference_size:header.networkconfig_size;
    char* buffer=NULL;
    if(bufferSize >0) buffer=(char*) malloc(bufferSize);

    uint32_t offset;
    noInterrupts();
    // erase sector
    bool eraseSuccess=(spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK);
    interrupts();
    if(! eraseSuccess)  goto END;
    DBG_PRINTF("Erase done.\n");
    // write header
    if(!_writeFlash((uint8_t*)&header, 0, sizeof(header))) goto END;
    offset = sizeof(header);
    DBG_PRINTF("Backup header done.\n");

    // write settings
    if(!_writeFlash((uint8_t*)FsEEPROM.data(), offset, header.setting_size)) goto END;
    offset += Round2Next4(header.setting_size); 
    DBG_PRINTF("Backup setting done:%lu\n",(long unsigned int)offset);

    // write network config
    if(header.networkconfig_size){
        File f=FileSystem.open(CONFIG_FILENAME,"r+");
        f.readBytes(buffer,header.networkconfig_size);
        f.close();
        if(!_writeFlash((uint8_t*)buffer,offset, header.networkconfig_size)) goto END;
        offset += Round2Next4(header.networkconfig_size); 
        DBG_PRINTF("Backup netcfg done:%lu\n",(long unsigned int)offset);
    }
    // write user preferences

    if(header.preference_size){
        File f=FileSystem.open(RECIPE_PREFERNECE,"r+");
        f.readBytes(buffer,header.preference_size);
        f.close();
        if(!_writeFlash((uint8_t*) buffer, offset, header.preference_size)) goto END;
         DBG_PRINTF("Backup netcfg done:%lu\n",(long unsigned int)(offset + header.preference_size));
    }
    // write user preferences
END:
    if(buffer) free(buffer);
}

void BackupServiceClass::restore(){
    BackupHeader header;
    _readFlash((uint8_t*) &header,0,sizeof(header));
    uint32_t offset=sizeof(header);

    if(header.pattern !=GuardPattern){
        DBG_PRINTF("guard pattern mismatched!\n");
        return;
    } 
    if(header.setting_size != (uint32_t)FsEEPROM.size()){
        DBG_PRINTF("setting size mismatched!\n");
        return;
    } 
    DBG_PRINTF("restore setting..\n");
    _readFlash((uint8_t*) FsEEPROM.data(),offset,header.setting_size);
    offset += Round2Next4(header.setting_size);
    FsEEPROM.commit(true);

    uint32_t bufferSize = (header.preference_size > header.networkconfig_size)? header.preference_size:header.networkconfig_size;
    char* buffer=NULL;
    if(bufferSize >0) buffer=(char*) malloc(bufferSize);

    if(header.networkconfig_size){
        DBG_PRINTF("restore netcfg:%lu..\n",(long unsigned int)header.networkconfig_size);

        _readFlash((uint8_t*) buffer,offset,header.networkconfig_size);
        offset += Round2Next4(header.networkconfig_size);
        File f=FileSystem.open(CONFIG_FILENAME,"w+");
        if(f){
            f.write((const uint8_t*)buffer,header.networkconfig_size);
            f.close();
        }else{
            DBG_PRINTF("failed to write netcfg.\n");
        }
    }
    if(header.preference_size){
        DBG_PRINTF("restore pref:%lu..\n",(long unsigned int)header.preference_size);

        _readFlash((uint8_t*) buffer,offset,header.preference_size);

        File f=FileSystem.open(RECIPE_PREFERNECE,"w+");
        if(f){
            f.write((const uint8_t*)buffer,header.preference_size);
            f.close();
        }else{
            DBG_PRINTF("failed to write pref\n");
        }
    }
    if(buffer) free(buffer);
}

#endif