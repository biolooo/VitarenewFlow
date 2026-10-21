#include "report_controls.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 assert(report_controls_hit(184,12)&&report_controls_hit(239,39));
 assert(!report_controls_hit(183,12)&&!report_controls_hit(240,39)&&!report_controls_hit(184,40));
 for(int x=244;x<274;x++)assert(!report_controls_hit(x,20)); /* Existing R remains reachable. */
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
