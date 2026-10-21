
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "app_log.c"
int main(void){char dir[]="/tmp/vf-logs-v128-XXXXXX";assert(mkdtemp(dir)&&!chdir(dir));mkdir("sdmc:",0700);mkdir("sdmc:/3ds",0700);mkdir("sdmc:/3ds/3ds-library",0700);app_log_begin();app_handoff_log("Handoff failure 1234");app_log_begin();FILE*f=fopen("sdmc:/3ds/3ds-library/handoff.log","rb");assert(f);char buf[2048]={0};fread(buf,1,sizeof(buf)-1,f);fclose(f);assert(strstr(buf,"Handoff failure 1234"));f=fopen("sdmc:/3ds/3ds-library/handoff.log","ab");assert(f);for(unsigned i=0;i<262145;i++)fputc('x',f);fclose(f);app_handoff_log("after rotation");struct stat st;assert(!stat("sdmc:/3ds/3ds-library/handoff.previous.log",&st)&&st.st_size>262144);assert(!stat("sdmc:/3ds/3ds-library/handoff.log",&st)&&st.st_size<100);puts("PASS persistent handoff log: failure survives new session, bounded rotation retains previous log");}
