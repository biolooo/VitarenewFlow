// SPDX-License-Identifier: GPL-3.0-only
#include "copy_result.h"
#include <stdio.h>
#include <string.h>
CopyResult copy_result_take(const char *path){
    FILE *file=fopen(path,"rb");
    if(!file)return COPY_NONE;
    char data[40];size_t count=fread(data,1,sizeof(data),file);
    int failed=ferror(file);fclose(file);
    if(failed||count==sizeof(data))return COPY_INCOMPLETE;
    data[count]=0;
    CopyResult result=COPY_INCOMPLETE;
    if(count==strlen(data)){
        if(!strcmp(data,"VF1 DS\n")||!strcmp(data,"VF1 DS\r\n"))result=COPY_DS_READY;
        if(!strcmp(data,"VF1 CIA\n")||!strcmp(data,"VF1 CIA\r\n"))result=COPY_CIA_READY;
    }
    /* A failed acknowledgement must never repeatedly announce success. */
    if(remove(path)!=0)return COPY_INCOMPLETE;
    return result;
}
