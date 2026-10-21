#include "cartridge.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
static bool inserted;static FS_CardType type;static int fault;
Result FSUSER_CardSlotIsInserted(bool*b){*b=inserted;return fault==1?-1:0;}
Result FSUSER_GetCardType(FS_CardType*t){*t=type;return fault==2?-2:0;}
Result AM_GetTitleList(u32*n,FS_MediaType m,u32 count,u64*id){assert(m==MEDIATYPE_GAME_CARD&&count==1);*n=fault==3?0:1;*id=0x000400000012BE00ULL;return 0;}
Result AM_GetTitleInfo(FS_MediaType m,u32 n,u64*id,AM_TitleInfo*out){(void)id;assert(m==MEDIATYPE_GAME_CARD&&n==1);out->size=1024;return 0;}
Result FSUSER_GetLegacyRomHeader(FS_MediaType m,u64 id,void *out){assert(m==MEDIATYPE_GAME_CARD&&id==0);unsigned char *p=out;memcpy(p,"DS GAME",7);memcpy(p+12,fault==5?"!!!!":"ABCE",4);p[0x82]=4;return fault==4?-4:0;}
Result FSUSER_GetLegacyBannerData(FS_MediaType m,u64 id,void*out){assert(m==MEDIATYPE_GAME_CARD&&id==0);unsigned char*p=out;p[0x440]='D';p[0x442]='E';p[0x340]='E';p[0x342]='N';p[0x20]=0x11;p[0x222]=31;return fault==6?-6:0;}
int main(void){Cartridge c;assert(!cartridge_read(&c,2)&&!c.present);inserted=true;type=CARD_CTR;assert(!cartridge_read(&c,2)&&c.present&&!c.ds&&c.id==0x000400000012BE00ULL&&c.size==1024);
 for(fault=1;fault<=3;fault++)assert(cartridge_read(&c,2)<0&&!c.present);
 fault=0;type=CARD_TWL;
 assert(!cartridge_read(&c,2)&&c.present&&c.ds&&c.size==262144&&!strcmp(c.code,"ABCE")&&!strcmp(c.name,"DE")&&c.hasIcon&&c.icon[0]==0xf800);
 assert(!cartridge_read(&c,1)&&!strcmp(c.name,"EN"));for(fault=4;fault<=5;fault++)assert(cartridge_read(&c,2)<0&&!c.present);
 fault=6;assert(!cartridge_read(&c,2)&&c.present&&!strcmp(c.name,"DS GAME")&&!c.hasIcon);inserted=false;assert(!cartridge_read(&c,2)&&!c.present);
 puts("PASS cartridge metadata: no card/removal, 3DS identity/size, DS header/title/locale/icon tiling, read failures and invalid header");}
