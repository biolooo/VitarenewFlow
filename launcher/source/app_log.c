// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "app_log.h"
#include <stdio.h>
#include <sys/stat.h>
void app_log_begin(void){FILE *f=fopen("sdmc:/3ds/3ds-library/session.log","wb");if(f){fputs("VitarenewFlow 0.12.38\n",f);fclose(f);}app_handoff_log("Session VitarenewFlow 0.12.38");}
void app_log(const char *message){FILE *f=fopen("sdmc:/3ds/3ds-library/session.log","ab");if(f){fprintf(f,"%s\n",message);fclose(f);}}
void app_perf_log(const char *message){app_log(message);FILE *f=fopen("sdmc:/3ds/3ds-library/performance.log","ab");if(f){fprintf(f,"%s\n",message);fclose(f);}}





void app_handoff_log(const char *message){
 const char *path="sdmc:/3ds/3ds-library/handoff.log",*old="sdmc:/3ds/3ds-library/handoff.previous.log";
 struct stat st;
 if(!stat(path,&st)&&st.st_size>262144){remove(old);if(rename(path,old))return;}
 FILE *f=fopen(path,"ab");if(f){fprintf(f,"%s\n",message);fclose(f);}
 app_log(message);
}
