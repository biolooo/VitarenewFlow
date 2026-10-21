// SPDX-License-Identifier: GPL-3.0-only
#include "update_details.h"
#include "release_notes_generated.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 assert(update_details_action(false,false,UPDATE_DETAILS_KEY_TOGGLE,0,0)==UPDATE_DETAILS_TOGGLE);
 assert(update_details_action(false,false,UPDATE_DETAILS_TOUCH,20,164)==UPDATE_DETAILS_TOGGLE);
 assert(update_details_action(false,false,UPDATE_DETAILS_TOUCH,299,187)==UPDATE_DETAILS_TOGGLE);
 assert(update_details_action(true,false,UPDATE_DETAILS_KEY_TOGGLE,0,0)==UPDATE_DETAILS_NONE);
 assert(update_details_action(false,false,UPDATE_DETAILS_KEY_BACK|UPDATE_DETAILS_KEY_TOGGLE,0,0)==UPDATE_DETAILS_BACK);
 assert(update_details_action(false,false,UPDATE_DETAILS_KEY_TOGGLE|UPDATE_DETAILS_KEY_PRIMARY,0,0)==UPDATE_DETAILS_TOGGLE);
 assert(update_details_action(false,true,UPDATE_DETAILS_KEY_PRIMARY,0,0)==UPDATE_DETAILS_NONE);
 assert(update_details_action(false,true,UPDATE_DETAILS_TOUCH,170,200)==UPDATE_DETAILS_NEXT);
 assert(update_details_action(false,true,UPDATE_DETAILS_TOUCH,20,200)==UPDATE_DETAILS_PREVIOUS);
 assert(update_details_action(false,false,UPDATE_DETAILS_TOUCH,170,200)==UPDATE_DETAILS_PRIMARY);
 assert(update_details_action(false,false,UPDATE_DETAILS_TOUCH,20,200)==UPDATE_DETAILS_BACK);
 assert(update_details_action(true,false,UPDATE_DETAILS_KEY_BACK,0,0)==UPDATE_DETAILS_BACK);
 assert(update_details_action(false,true,UPDATE_DETAILS_TOUCH,280,12)==UPDATE_DETAILS_BACK);
 assert(update_details_action(false,false,UPDATE_DETAILS_TOUCH,19,164)==UPDATE_DETAILS_NONE);
 UpdateStatus status={0};
 const char *installed=flow_release_notes.version;
 const UpdateNotes *local=update_details_notes(&status,installed);
 assert(local&&local->count>=1&&local->count<=3&&!strcmp(local->version,installed)&&local->en[0][0]);
 strcpy(status.available,"0.12.99");
 assert(!update_details_notes(&status,installed)); /* Never show installed text as future changes. */
 assert(!strcmp(update_details_version(&status,installed),"0.12.99"));
 status.notes_state=UPDATE_NOTES_AVAILABLE;status.notes.count=1;
 strcpy(status.notes.version,"0.12.99");strcpy(status.notes.de[0],"Neue Funktion");strcpy(status.notes.en[0],"New feature");
 assert(update_details_notes(&status,installed)==&status.notes); /* Future version needs no client update. */
 strcpy(status.notes.version,"0.12.98");
 assert(!update_details_notes(&status,installed)); /* stale or mismatched snapshot */
 assert(!update_details_notes(NULL,"0.12.99"));
 puts("PASS: X/touch, close priority, busy lockout, paging without installation, future-version notes and stale-target rejection");
 return 0;
}
