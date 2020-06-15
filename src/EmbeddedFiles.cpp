#include <Arduino.h>
#include <pgmspace.h>
#include "config.h"

#ifndef NULL
#define NULL 0
#endif

#define STRINGIFY(str)  #str
#define PASTER(lo,file)   STRINGIFY(lo ## file )
#define EVALUATOR(l,x)  PASTER(l,x)


#define BmHtmFile EVALUATOR(WebPageLanguage,_bm_htm.h)


#if NoEmbeddedFile == true

const uint8_t* getEmbeddedFile(const char* filename,bool &gzip, unsigned int &size){
	return NULL;
}

#else

typedef struct _EmbeddedFileMapEntry{
	const char *filename;
	const uint8_t *content;
    unsigned int size;
	bool  gzipped;
} EmbeddedFileMapEntry;



#include BmHtmFile

const char file_bm_htm [] PROGMEM="/bm.htm";

EmbeddedFileMapEntry fileMaps[]={
{file_bm_htm,data_bm_htm_gz,sizeof(data_bm_htm_gz),true},
};



const uint8_t* getEmbeddedFile(const char* filename,bool &gzip, unsigned int &size)
{
	for(int i=0;i<(int)(sizeof(fileMaps)/sizeof(EmbeddedFileMapEntry));i++)
	{
		if(strcmp_P(filename,fileMaps[i].filename) ==0){
		    gzip = fileMaps[i].gzipped;
		    size = fileMaps[i].size;
			return fileMaps[i].content;
		}
	}
	return NULL;
}
#endif