// SPDX-License-Identifier: GPL-3.0-only
#include "report_history.h"
#include <string.h>
void report_history_record(ReportHistory *h,ErrorReportActivity a){
 if(!h||(unsigned)a>=ERROR_REPORT_ACTIVITY_COUNT)return;
 h->generation++;
 if(h->count&&h->events[h->count-1]==a)return;
 if(h->count==ERROR_REPORT_TRACE_MAX){memmove(h->events,h->events+1,(ERROR_REPORT_TRACE_MAX-1)*sizeof(*h->events));h->count--;}
 h->events[h->count++]=a;
}
void report_history_snapshot(const ReportHistory *h,ErrorReportDiagnostics *s){
 if(!s)return;
 s->trace_count=0;memset(s->trace,0,sizeof(s->trace));
 if(!h||h->count>ERROR_REPORT_TRACE_MAX)return;
 s->trace_count=h->count;memcpy(s->trace,h->events,h->count*sizeof(*h->events));
}
