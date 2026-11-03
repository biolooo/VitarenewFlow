// SPDX-License-Identifier: GPL-3.0-only
/* Include the real implementation to check bsearch's entire sorted-table contract. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "l10n.c"
int main(void){
 for(unsigned i=1;i<sizeof(translations)/sizeof(translations[0]);i++)assert(strcmp(translations[i-1].de,translations[i].de)<0);
 const char *de[]={"Desktop wird auf SD gespeichert","Windows, macOS oder Linux auf SD laden","Archiv dieser Plattform auf SD vorhanden. Am Computer entpacken.","Kein Archiv dieser Plattform auf der SD gefunden.","Archiv auf SD gespeichert. Am Computer entpacken und Desktop-App öffnen.","Desktop-Verwaltung","Emulatoren","Zurück"};
 const char *en[]={"Saving Desktop to SD","Download Windows, macOS or Linux to SD","Archive for this platform on SD. Extract on your computer.","No archive for this platform found on SD.","Archive saved to SD. Extract on your computer and open Desktop.","Desktop manager","Emulators","Back"};
 l10n_load(true);assert(l10n_english());for(unsigned i=0;i<sizeof(de)/sizeof(de[0]);i++)assert(!strcmp(tr(de[i]),en[i]));
 l10n_load(false);assert(!l10n_english());for(unsigned i=0;i<sizeof(de)/sizeof(de[0]);i++)assert(!strcmp(tr(de[i]),de[i]));
 puts("PASS: actual l10n bsearch sorted-table contract, five new Desktop labels and established DE/EN labels");return 0;
}
