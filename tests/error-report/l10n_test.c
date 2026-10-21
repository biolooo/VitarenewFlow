#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "l10n.c"
int main(void){
 english=true;
 for(size_t i=0;i<sizeof(translations)/sizeof(*translations);i++){
  if(i)assert(strcmp(translations[i-1].de,translations[i].de)<0);
  assert(!strcmp(tr(translations[i].de),translations[i].en));
 }
 assert(!strcmp(tr("Problem melden"),"Report a problem"));
 assert(!strcmp(tr("A Jetzt senden"),"A Send now"));
 english=false;assert(!strcmp(tr("Problem melden"),"Problem melden"));
 puts("PASS: every actual bsearch translation, report DE/EN, sorted unique table");return 0;
}
