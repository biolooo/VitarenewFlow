// SPDX-License-Identifier: GPL-3.0-only
#include "update_store.h"
int update_store_commit(const UpdateFs *fs){
 if(!fs||!fs->exists(UPDATE_APP)||!fs->exists(UPDATE_STAGE))return -1;
 if(fs->exists(UPDATE_BACKUP)&&fs->erase(UPDATE_BACKUP))return -1;
 if(fs->move(UPDATE_APP,UPDATE_BACKUP))return -1;
 if(fs->move(UPDATE_STAGE,UPDATE_APP))return fs->move(UPDATE_BACKUP,UPDATE_APP)?-2:-1;
 return 0;
}
