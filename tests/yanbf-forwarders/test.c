// SPDX-License-Identifier: GPL-3.0-only
#include "library_model.h"
#include "l10n.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 const uint64_t low=0x000400000FF40000ULL,high=0x000400000FF7FF00ULL;
 assert(library_is_yanbf_forwarder(low,"CTR-H-ABCD")&&library_is_yanbf_forwarder(high,"CTR-H-A2EJ"));
 assert(library_is_yanbf_forwarder(low+0x100,"CTR-H-####")&&library_is_yanbf_forwarder(low,"CTR-H-ab12"));
 assert(!library_is_yanbf_forwarder(low-0x100,"CTR-H-ABCD")&&!library_is_yanbf_forwarder(high+0x100,"CTR-H-ABCD"));
 assert(!library_is_yanbf_forwarder(low+1,"CTR-H-ABCD")&&!library_is_yanbf_forwarder(0x000480040FF40000ULL,"CTR-H-ABCD"));
 const char *bad[]={NULL,"","CTR-H-ABC","CTR-H-ABCDE","CTR-P-ABCD","CTR-HB-ABC","CTR-H-A CD","CTR-H-\x80""BCD","CTR-H-AB\0D"};
 for(unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);i++)assert(!library_is_yanbf_forwarder(low,bad[i]));
 assert(!library_is_yanbf_forwarder(0x00040000001A1E00ULL,"CTR-H-ABCD"));
 assert(library_is_homebrew(low,"CTR-H-ABCD"));
 assert(library_title_view(SYSTEM_3DS,true,true)==VIEW_DS);
 assert(library_title_view(SYSTEM_3DS,true,false)==VIEW_HOMEBREW);
 assert(library_title_view(SYSTEM_3DS,false,false)==VIEW_3DS);
 assert(library_title_view(SYSTEM_DS,false,false)==VIEW_DS);
 assert(library_title_display_system(SYSTEM_3DS,true)==SYSTEM_DS&&library_title_display_system(SYSTEM_3DS,false)==SYSTEM_3DS);
 l10n_load(true);assert(!strcmp(tr("DS HOME-Verknüpfung"),"DS HOME shortcut"));
 puts("PASS: actual YANBF dual-marker classifier, boundaries/ASCII/invalid IDs, DS-only presentation, unchanged homebrew/retail views and English label");return 0;
}
