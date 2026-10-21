// SPDX-License-Identifier: GPL-3.0-only
/* HOME extdata layout documented by Anemone3DS include/themes.h. Independent reader. */
#include "theme_music_plan.h"
#include <string.h>
static uint32_t u32(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
bool theme_music_plan(ThemeMusicPlan *p,const uint8_t *s,size_t sn,const uint8_t *m,size_t mn){
 if(!p)return false;
 memset(p,0,sizeof(*p));if(!m||mn<0x388||!u32(m+28))return false;
 p->known=s&&sn>=0x141c;
 if(p->known){p->shuffle=s[0x141b]!=0;if(!p->shuffle&&s[0x13bd]!=3)return false;}
 if(p->shuffle){p->slot=u32(s+0x13b8);if(p->slot>=10)return false;p->bodyBytes=u32(m+0x338+4*p->slot);p->musicBytes=u32(m+0x360+4*p->slot);}
 else {p->bodyBytes=u32(m+8);p->musicBytes=u32(m+12);}
 return p->bodyBytes>=4&&p->bodyBytes<=0x150000&&p->musicBytes>=12&&p->musicBytes<=3371008;
}
