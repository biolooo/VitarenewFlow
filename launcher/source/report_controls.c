// SPDX-License-Identifier: GPL-3.0-only
#include "report_controls.h"
bool report_controls_hit(int x,int y){return x>=184&&x<240&&y>=12&&y<40;}
ReportAction report_controls_action(ReportSendState state,bool configured,bool confirmed,bool back,bool primary,bool edit){
 if(back)return state==REPORT_SENDING?REPORT_ACTION_CANCEL:REPORT_ACTION_CLOSE;
 if(state==REPORT_SENDING||state==REPORT_SENT)return REPORT_ACTION_NONE;
 if(edit)return REPORT_ACTION_EDIT;
 if(primary&&configured)return confirmed?REPORT_ACTION_SEND:REPORT_ACTION_REVIEW;
 return REPORT_ACTION_NONE;
}
