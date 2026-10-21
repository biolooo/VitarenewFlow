// SPDX-License-Identifier: GPL-3.0-only
#include "forwarder_restore.h"
#include <string.h>
bool forwarder_restore_parse(const char *arg,uint64_t *title){
 const char *prefix="vitarenewflow-forwarder:restore=";
 if(!arg||!title||strncmp(arg,prefix,strlen(prefix)))return false;
 arg+=strlen(prefix);if(strlen(arg)!=16)return false;
 uint64_t value=0;for(unsigned i=0;i<16;i++){
  unsigned c=(unsigned char)arg[i],n=c>='0'&&c<='9'?c-'0':c>='A'&&c<='F'?c-'A'+10:c>='a'&&c<='f'?c-'a'+10:16;
  if(n>15)return false;
  value=value*16+n;
 }
 if((value>>32)!=0x00040000ULL&&(value>>32)!=0x00040010ULL)return false;
 if(value==0x000400000F104200ULL)return false;
 *title=value;return true;
}
