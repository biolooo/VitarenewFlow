// SPDX-License-Identifier: GPL-3.0-only
#include "update_details.h"
#include "release_notes_generated.h"
#include <string.h>
UpdateDetailsAction update_details_action(bool busy,bool opened,unsigned input,int x,int y){
 bool touch=(input&UPDATE_DETAILS_TOUCH)!=0;
 if((input&UPDATE_DETAILS_KEY_BACK)||(touch&&x>=276&&x<312&&y>=8&&y<44))return UPDATE_DETAILS_BACK;
 if(!busy&&((input&UPDATE_DETAILS_KEY_TOGGLE)||(touch&&!opened&&x>=20&&x<300&&y>=164&&y<188)))return UPDATE_DETAILS_TOGGLE;
 if(opened){
  if((input&UPDATE_DETAILS_KEY_PREVIOUS)||(touch&&x>=12&&x<151&&y>=192&&y<230))return UPDATE_DETAILS_PREVIOUS;
  if((input&UPDATE_DETAILS_KEY_NEXT)||(touch&&x>=169&&x<308&&y>=192&&y<230))return UPDATE_DETAILS_NEXT;
  return UPDATE_DETAILS_NONE;
 }
 if(touch&&x>=12&&x<151&&y>=192&&y<230)return UPDATE_DETAILS_BACK;
 if(!busy&&((input&UPDATE_DETAILS_KEY_PRIMARY)||(touch&&x>=169&&x<308&&y>=192&&y<230)))return UPDATE_DETAILS_PRIMARY;
 return UPDATE_DETAILS_NONE;
}
const char *update_details_version(const UpdateStatus *status,const char *installed){
 return status&&status->available[0]?status->available:installed;
}
const UpdateNotes *update_details_notes(const UpdateStatus *status,const char *installed){
 const char *target=update_details_version(status,installed);
 if(!target)return NULL;
 if(status&&status->notes_state==UPDATE_NOTES_AVAILABLE&&status->notes.count&&
    !strcmp(status->notes.version,target))return &status->notes;
 if(installed&&!strcmp(target,installed)&&!strcmp(installed,flow_release_notes.version))return &flow_release_notes;
 return NULL;
}
