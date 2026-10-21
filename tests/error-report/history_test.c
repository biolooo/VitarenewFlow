#include "report_history.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void){
 ReportHistory h={0};ErrorReportDiagnostics s={0};
 report_history_record(NULL,ERROR_REPORT_BROWSING);report_history_snapshot(NULL,&s);assert(!s.trace_count);
 report_history_record(&h,ERROR_REPORT_BROWSING);report_history_record(&h,ERROR_REPORT_BROWSING);
 assert(h.generation==2&&h.count==1);
 for(unsigned i=1;i<ERROR_REPORT_ACTIVITY_COUNT;i++)report_history_record(&h,(ErrorReportActivity)i);
 assert(h.count==ERROR_REPORT_TRACE_MAX);report_history_snapshot(&h,&s);
 assert(s.trace_count==ERROR_REPORT_TRACE_MAX&&s.trace[7]==ERROR_REPORT_IMPORTING);
 uint32_t before=h.generation;report_history_record(&h,(ErrorReportActivity)-1);assert(h.generation==before);
 ErrorReportDiagnostics preserved=s;report_history_record(&h,ERROR_REPORT_BROWSING);
 assert(!memcmp(&s,&preserved,sizeof(s))); /* Frozen incident survives later navigation. */
 report_history_snapshot(&h,&s);assert(s.trace[7]==ERROR_REPORT_BROWSING);
 puts("PASS: bounded chronological activity ring, coalescing, invalid categories and frozen incident lifetime");return 0;
}
