#include "report_controls.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 assert(report_controls_hit(190,12)&&report_controls_hit(269,39));
 assert(!report_controls_hit(189,12)&&!report_controls_hit(270,39)&&!report_controls_hit(190,40));
 assert(report_controls_support_hit(12,170)&&report_controls_support_hit(307,199));
 assert(!report_controls_support_hit(12,205)&&!report_controls_support_hit(308,180));
 assert(!report_controls_fallback(true,false,false)); /* Ordinary R/header never intercepted. */
 assert(report_controls_fallback(true,true,false)&&report_controls_fallback(true,false,true));
 assert(!report_controls_fallback(false,true,true)); /* Critical system/installation state wins. */
 for(int x=278;x<312;x++)assert(!report_controls_hit(x,20)); /* Settings/close untouched. */
 assert(report_controls_action(REPORT_IDLE,true,false,false,true,false)==REPORT_ACTION_REVIEW);
 assert(report_controls_action(REPORT_IDLE,true,true,false,true,false)==REPORT_ACTION_SEND);
 assert(report_controls_action(REPORT_FAILED,true,false,false,true,false)==REPORT_ACTION_REVIEW);
 assert(report_controls_action(REPORT_SENDING,true,true,true,true,true)==REPORT_ACTION_CANCEL);
 assert(report_controls_action(REPORT_SENT,true,true,true,true,true)==REPORT_ACTION_CLOSE);
 assert(report_controls_action(REPORT_IDLE,true,true,false,true,true)==REPORT_ACTION_EDIT);
 for(unsigned state=0;state<=REPORT_CANCELLED;state++)for(unsigned flags=0;flags<8;flags++){
  bool back=flags&1,primary=flags&2,edit=flags&4;
  assert(report_controls_action(state,false,true,back,primary,edit)!=REPORT_ACTION_SEND);
  assert(report_controls_action(state,true,false,back,primary,edit)!=REPORT_ACTION_SEND);
  ReportAction a=report_controls_action(state,true,true,back,primary,edit);
  if(state==REPORT_SENDING)assert(a==REPORT_ACTION_CANCEL||a==REPORT_ACTION_NONE);
  if(state==REPORT_SENT)assert(a==REPORT_ACTION_CLOSE||a==REPORT_ACTION_NONE);
  if(a==REPORT_ACTION_SEND)assert(primary&&!back&&!edit&&state!=REPORT_SENT&&state!=REPORT_SENDING);
 }
 puts("PASS: actual modal consent/review/edit/back priorities, busy and success lockout");return 0;
}
