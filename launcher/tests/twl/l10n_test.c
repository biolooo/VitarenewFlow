#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "l10n.c"
int main(void){l10n_load(true);assert(l10n_english());for(unsigned i=0;i<sizeof(translations)/sizeof(translations[0]);i++){if(i)assert(strcmp(translations[i-1].de,translations[i].de)<0);assert(!strcmp(tr(translations[i].de),translations[i].en));}l10n_load(false);assert(!strcmp(tr("DS-Diagnose und Reparatur"),"DS-Diagnose und Reparatur"));puts("PASS actual DE/EN runtime lookup for every translation, sorted binary search and language switching");}
