#include "app_log.h"
#include <stdio.h>
void app_log_begin(void){FILE *f=fopen("sdmc:/3ds/3ds-library/session.log","wb");if(f){fputs("VitarenewFlow 0.11.0\n",f);fclose(f);}}
void app_log(const char *message){FILE *f=fopen("sdmc:/3ds/3ds-library/session.log","ab");if(f){fprintf(f,"%s\n",message);fclose(f);}}
void app_perf_log(const char *message){app_log(message);FILE *f=fopen("sdmc:/3ds/3ds-library/performance.log","ab");if(f){fprintf(f,"%s\n",message);fclose(f);}}




