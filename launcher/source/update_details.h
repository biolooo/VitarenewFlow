// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "app_update.h"
typedef enum {UPDATE_DETAILS_NONE,UPDATE_DETAILS_BACK,UPDATE_DETAILS_PRIMARY,
 UPDATE_DETAILS_TOGGLE,UPDATE_DETAILS_PREVIOUS,UPDATE_DETAILS_NEXT} UpdateDetailsAction;
enum {UPDATE_DETAILS_KEY_BACK=1,UPDATE_DETAILS_KEY_PRIMARY=2,UPDATE_DETAILS_KEY_TOGGLE=4,
 UPDATE_DETAILS_KEY_PREVIOUS=8,UPDATE_DETAILS_KEY_NEXT=16,UPDATE_DETAILS_TOUCH=32};
UpdateDetailsAction update_details_action(bool busy,bool opened,unsigned input,int x,int y);
const UpdateNotes *update_details_notes(const UpdateStatus *status,const char *installed);
const char *update_details_version(const UpdateStatus *status,const char *installed);
