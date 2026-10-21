// SPDX-License-Identifier: GPL-3.0-only
#include "twl_advice.h"
TwlAdvice twl_advice(bool known,bool supported,unsigned missing,bool loader,bool files,bool allFail,bool downloadPlayFail){
 if(!known)return TWL_CHECK_UNKNOWN;
 if(!supported)return TWL_CHECK_UNSUPPORTED;
 /* Missing core titles can be restored by an update without first deleting
    the remaining titles. Missing Flow files are a setup issue, not TWL damage. */
 if(missing)return TWL_CHECK_UPDATE;
 if(!loader||!files)return TWL_CHECK_LOADER;
 return allFail&&downloadPlayFail?TWL_CHECK_REPAIR:TWL_CHECK_CLEAR;
}
