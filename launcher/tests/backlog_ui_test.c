// SPDX-License-Identifier: GPL-3.0-only
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "settings_menu.h"
#include "l10n.h"
#include "update_format.h"

int main(void){
    /* Removing a menu row must also remove its old touch target. */
    assert(settings_menu_count(MENU_PERSONAL)==2);
    assert(settings_menu_item(MENU_PERSONAL,0)->action==MENU_LANGUAGE);
    assert(settings_menu_item(MENU_PERSONAL,1)->action==MENU_THEME);
    assert(settings_menu_item(MENU_PERSONAL,2)==NULL);
    assert(settings_menu_hit(160,54,MENU_PERSONAL)==0);
    assert(settings_menu_hit(160,100,MENU_PERSONAL)==1);
    assert(settings_menu_hit(160,146,MENU_PERSONAL)==-1);
    assert(settings_menu_hit(160,94,MENU_PERSONAL)==-1);
    assert(settings_menu_hit(311,100,MENU_PERSONAL)==-1);
    assert(settings_menu_parent_row(MENU_PERSONAL)==0);
    assert(settings_release_note_count(NULL)==0);
    assert(settings_release_note_count("0.12.37")==0);
    assert(settings_release_note("0.12.37",0)==NULL);
    assert(settings_release_note_count("0.12.33")==3);
    assert(settings_release_note("0.12.33",3)==NULL);
    assert(settings_release_note_count("0.12.34")==3);
    assert(settings_release_note_count("0.12.35")==3);
    assert(settings_release_note_count("0.12.36")==3);
    assert(settings_release_note("0.12.35",3)==NULL);
    assert(settings_release_note("0.12.34",3)==NULL);
    assert(strcmp(settings_release_note("0.12.33",0),settings_release_note("0.12.34",0)));
    assert(update_version_compare("0.12.33","0.12.34")<0);
    assert(update_version_compare("0.12.34","0.12.34")==0);
    assert(update_version_compare("0.12.35","0.12.34")>0);
    assert(update_version_compare("0.12.100","0.12.34")>0);
    assert(settings_update_notes_hit(20,172));
    assert(settings_update_notes_hit(299,186));
    assert(!settings_update_notes_hit(19,172));
    assert(!settings_update_notes_hit(300,172));
    assert(!settings_update_notes_hit(20,171));
    assert(!settings_update_notes_hit(20,187));
    assert(!settings_update_notes_hit(169,192)); /* Install button is separate. */
    for(unsigned page=MENU_ROOT;page<=MENU_SYSTEM;page++){
        for(unsigned row=0;row<settings_menu_count(page);row++){
            const MenuItem *item=settings_menu_item(page,row);
            assert(item&&item->label&&item->detail);
            assert(settings_menu_hit(100,54+46*row,page)==(int)row);
        }
    }
    /* Production bsearch must find every new visible phrase in English. */
    const char *de[]={"Sprache und Theme","Spielstart","Installierte Version: Neuerungen",
        "- Desktop erkennen und reparieren","- DS-Laufzeit erneut prüfen",
        "- Suche mit Wortvorschlägen","X Neuerungen einklappen",
        "X Neuerungen dieser Version","Entwicklungsstand","Von VitaRenew für deine Sammlung."};
    l10n_load(true);
    for(unsigned i=0;i<sizeof(de)/sizeof(de[0]);i++)assert(strcmp(tr(de[i]),de[i]));
    for(unsigned i=0;i<settings_release_note_count("0.12.33");i++)assert(strcmp(tr(settings_release_note("0.12.33",i)),settings_release_note("0.12.33",i)));
    for(unsigned i=0;i<settings_release_note_count("0.12.36");i++)assert(strcmp(tr(settings_release_note("0.12.36",i)),settings_release_note("0.12.36",i)));
    for(unsigned i=0;i<settings_release_note_count("0.12.35");i++)assert(strcmp(tr(settings_release_note("0.12.35",i)),settings_release_note("0.12.35",i)));
    for(unsigned i=0;i<settings_release_note_count("0.12.34");i++)assert(strcmp(tr(settings_release_note("0.12.34",i)),settings_release_note("0.12.34",i)));
    l10n_load(false);
    for(unsigned i=0;i<sizeof(de)/sizeof(de[0]);i++)assert(!strcmp(tr(de[i]),de[i]));
    puts("PASS: personal menu actions/touch boundaries, versioned DE/EN release highlights and .34 update comparisons");
    return 0;
}
