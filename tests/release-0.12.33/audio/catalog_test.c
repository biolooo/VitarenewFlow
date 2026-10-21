#include "component_catalog.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void){for(unsigned n=0;n<COMPONENT_COUNT;n++){const ComponentDef *d=component_definition(n);assert(d&&d->name&&d->version&&d->count&&d->count<=COMPONENT_MAX_FILES);for(unsigned i=0;i<d->count;i++){assert(d->files[i].bytes&&strlen(d->files[i].sha)==64);assert(!strncmp(d->files[i].target,"sdmc:/",6)&&!strstr(d->files[i].target,".."));assert(!strncmp(d->files[i].url,"https://biolooo.github.io/VitarenewFlow/updates/",44));}}assert(!component_definition(COMPONENT_COUNT));puts("PASS: all six emulator packages and Desktop have bounded fixed HTTPS/SD catalogs");}
