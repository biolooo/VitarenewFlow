// SPDX-License-Identifier: GPL-3.0-only
#include "forwarder_restore.h"
#include <3ds.h>
#include "luma_shared_config.h"
void forwarder_restore(int argc,char **argv){
 uint64_t previous=0;s64 luma=0;
 if(!envIsHomebrew()||argc!=2||!forwarder_restore_parse(argv[1],&previous)||R_FAILED(svcGetSystemInfo(&luma,0x10000,0)))return;
 /* Only consume an explicit shortcut handoff, never overwrite unrelated state. */
 if(Luma_SharedConfig->hbldr_3dsx_tid==0x000400000F104200ULL&&Luma_SharedConfig->selected_hbldr_3dsx_tid==0x000400000F104200ULL)
  Luma_SharedConfig->selected_hbldr_3dsx_tid=previous;
}
