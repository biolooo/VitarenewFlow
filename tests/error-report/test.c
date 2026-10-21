// SPDX-License-Identifier: GPL-3.0-only
#include "error_report.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static ErrorReport valid_report(void){
    ErrorReport r={.version="0.12.34",.area=ERROR_REPORT_LAUNCH,.code=INT32_MIN,
        .model=ERROR_REPORT_NEW_3DS_XL,.region=ERROR_REPORT_EUR,
        .autoboot=ERROR_REPORT_AUTOBOOT_ON,.context=ERROR_REPORT_HOMEBREW,
        .activity=ERROR_REPORT_STARTING_GAME};
    return r;
}
static void rejected(ErrorReport *r){
    char out[ERROR_REPORT_JSON_CAPACITY];strcpy(out,"old payload");size_t n=123;
    assert(!error_report_json(r,out,sizeof(out),&n));assert(!out[0]&&!n);
}
int main(void){
    ErrorReport r=valid_report();char out[ERROR_REPORT_JSON_CAPACITY];size_t n=0;
    assert(error_report_json(&r,out,sizeof(out),&n));assert(n==strlen(out));
    assert(strstr(out,"\"code\":-2147483648"));puts(out);
    r.code=INT32_MAX;r.description="He said \"Hallo\".\\\n\t\r\b\f\x01 Gr\xc3\xbc\xc3\x9f" "e \xf0\x9f\x8e\xae";
    assert(error_report_json(&r,out,sizeof(out),&n));puts(out);
    size_t required=n+1;
    for(size_t capacity=1;capacity<=required;capacity++){
        unsigned char guarded[ERROR_REPORT_JSON_CAPACITY+2];memset(guarded,0xa5,sizeof(guarded));
        size_t written=99;bool ok=error_report_json(&r,(char*)guarded+1,capacity,&written);
        assert(guarded[0]==0xa5&&guarded[capacity+1]==0xa5);
        if(capacity<required)assert(!ok&&!guarded[1]&&!written);
        else assert(ok&&written==required-1&&!memcmp(guarded+1,out,required));
    }
    char tiny='Q';assert(!error_report_json(&r,&tiny,0,&n)&&tiny=='Q'&&!n);
    assert(!error_report_json(&r,NULL,100,&n)&&!n);
    assert(!error_report_json(NULL,out,sizeof(out),&n)&&!out[0]&&!n);
    assert(error_report_json(&r,out,sizeof(out),NULL));
    const char *invalid_versions[]={NULL,"","1","1.2","1.2.3.4","1..2",".1.2","1.2.","1.2.-3","sdmc:/secret","1.2.3\n","12345678901234567890123456789012.1.2"};
    for(size_t i=0;i<sizeof(invalid_versions)/sizeof(invalid_versions[0]);i++){r=valid_report();r.version=invalid_versions[i];rejected(&r);}
    r=valid_report();r.area=(ErrorReportArea)-1;rejected(&r);
    r=valid_report();r.area=ERROR_REPORT_AREA_COUNT;rejected(&r);
    r=valid_report();r.model=ERROR_REPORT_MODEL_COUNT;rejected(&r);
    r=valid_report();r.region=ERROR_REPORT_REGION_COUNT;rejected(&r);
    r=valid_report();r.autoboot=ERROR_REPORT_AUTOBOOT_COUNT;rejected(&r);
    r=valid_report();r.context=ERROR_REPORT_CONTEXT_COUNT;rejected(&r);
    r=valid_report();r.activity=ERROR_REPORT_ACTIVITY_COUNT;rejected(&r);
    const char *invalid_utf8[]={"\xc0\x80","\xc2","\xe0\x80\x80","\xed\xa0\x80","\xf4\x90\x80\x80","\xf0\x80\x80\x80","\xff","\x80","\xe2\x28\xa1"};
    for(size_t i=0;i<sizeof(invalid_utf8)/sizeof(invalid_utf8[0]);i++){r=valid_report();r.description=invalid_utf8[i];rejected(&r);}
    char description[ERROR_REPORT_DESCRIPTION_MAX+2];memset(description,1,ERROR_REPORT_DESCRIPTION_MAX);description[ERROR_REPORT_DESCRIPTION_MAX]=0;
    r=valid_report();r.description=description;assert(error_report_json(&r,out,sizeof(out),&n));assert(n<ERROR_REPORT_JSON_CAPACITY);puts(out);
    description[ERROR_REPORT_DESCRIPTION_MAX]='!';description[ERROR_REPORT_DESCRIPTION_MAX+1]=0;rejected(&r);
    memset(description,'a',ERROR_REPORT_DESCRIPTION_MAX-1);description[ERROR_REPORT_DESCRIPTION_MAX-1]=(char)0xc3;description[ERROR_REPORT_DESCRIPTION_MAX]=0;rejected(&r);
    /* Cover all technical categories; output values remain closed strings. */
    for(unsigned i=0;i<ERROR_REPORT_ACTIVITY_COUNT;i++){
        r=valid_report();r.area=(ErrorReportArea)(i%ERROR_REPORT_AREA_COUNT);r.model=(ErrorReportModel)(i%ERROR_REPORT_MODEL_COUNT);
        r.region=(ErrorReportRegion)(i%ERROR_REPORT_REGION_COUNT);r.autoboot=(ErrorReportAutoboot)(i%ERROR_REPORT_AUTOBOOT_COUNT);
        r.context=(ErrorReportContext)(i%ERROR_REPORT_CONTEXT_COUNT);r.activity=(ErrorReportActivity)i;
        assert(error_report_json(&r,out,sizeof(out),&n));puts(out);
    }
    /* Deterministic escaping fuzz: arbitrary non-NUL ASCII input, bounded. */
    unsigned state=789;
    for(unsigned trial=0;trial<200;trial++){
        unsigned length=trial%129;
        for(unsigned i=0;i<length;i++){state=state*1664525u+1013904223u;description[i]=(char)(1+(state%127));}
        description[length]=0;r=valid_report();r.description=description;
        assert(error_report_json(&r,out,sizeof(out),&n));puts(out);
    }
    /* NULL is still exactly schema 1; an explicit empty snapshot is schema 2. */
    ErrorReportDiagnostics d={0};r=valid_report();r.diagnostics=&d;
    assert(error_report_json(&r,out,sizeof(out),&n));assert(strstr(out,"\"schema\":2"));puts(out);
    d=(ErrorReportDiagnostics){.uptime_seconds=ERROR_REPORT_UPTIME_MAX,.sd_free_mib=ERROR_REPORT_SD_FREE_MAX,
        .linear_free_kib=ERROR_REPORT_LINEAR_FREE_MAX,.validity=7,.trace_count=ERROR_REPORT_TRACE_MAX,
        .trace={ERROR_REPORT_BROWSING,ERROR_REPORT_CHECKING_UPDATE,ERROR_REPORT_DOWNLOADING,
                ERROR_REPORT_CHECKING_COMPONENTS,ERROR_REPORT_CHECKING_DS,ERROR_REPORT_OPENING_HOME,
                ERROR_REPORT_LOADING_THEME,ERROR_REPORT_STARTING_GAME}};
    memset(description,1,ERROR_REPORT_DESCRIPTION_MAX);description[ERROR_REPORT_DESCRIPTION_MAX]=0;r.description=description;
    assert(error_report_json(&r,out,sizeof(out),&n));assert(n<ERROR_REPORT_JSON_CAPACITY);puts(out);
    required=n+1;
    for(size_t capacity=1;capacity<=required;capacity++){
        unsigned char guarded[ERROR_REPORT_JSON_CAPACITY+2];memset(guarded,0xa5,sizeof(guarded));size_t written=99;
        bool ok=error_report_json(&r,(char*)guarded+1,capacity,&written);
        assert(guarded[0]==0xa5&&guarded[capacity+1]==0xa5);
        if(capacity<required)assert(!ok&&!guarded[1]&&!written);
        else assert(ok&&written==required-1&&!memcmp(guarded+1,out,required));
    }
    /* Every validity combination is explicit; unknown counters cannot leak. */
    r.description=NULL;
    for(unsigned flags=0;flags<8;flags++){
        d.validity=(uint8_t)flags;d.uptime_seconds=(flags&1)?ERROR_REPORT_UPTIME_MAX:0;
        d.sd_free_mib=(flags&2)?ERROR_REPORT_SD_FREE_MAX:0;d.linear_free_kib=(flags&4)?ERROR_REPORT_LINEAR_FREE_MAX:0;
        assert(error_report_json(&r,out,sizeof(out),&n));puts(out);
    }
    for(unsigned activity=0;activity<ERROR_REPORT_ACTIVITY_COUNT;activity++){
        d=(ErrorReportDiagnostics){.trace_count=1,.trace={(ErrorReportActivity)activity}};
        assert(error_report_json(&r,out,sizeof(out),&n));puts(out);
    }
    d=(ErrorReportDiagnostics){.validity=8};rejected(&r);
    d=(ErrorReportDiagnostics){.validity=255};rejected(&r);
    d=(ErrorReportDiagnostics){.validity=1,.uptime_seconds=ERROR_REPORT_UPTIME_MAX+1};rejected(&r);
    d=(ErrorReportDiagnostics){.validity=2,.sd_free_mib=ERROR_REPORT_SD_FREE_MAX+1};rejected(&r);
    d=(ErrorReportDiagnostics){.validity=4,.linear_free_kib=ERROR_REPORT_LINEAR_FREE_MAX+1};rejected(&r);
    d=(ErrorReportDiagnostics){.validity=7,.uptime_seconds=UINT32_MAX};rejected(&r);
    d=(ErrorReportDiagnostics){.validity=7,.sd_free_mib=UINT32_MAX};rejected(&r);
    d=(ErrorReportDiagnostics){.validity=7,.linear_free_kib=UINT32_MAX};rejected(&r);
    d=(ErrorReportDiagnostics){.uptime_seconds=1};rejected(&r);
    d=(ErrorReportDiagnostics){.sd_free_mib=1};rejected(&r);
    d=(ErrorReportDiagnostics){.linear_free_kib=1};rejected(&r);
    d=(ErrorReportDiagnostics){.trace_count=ERROR_REPORT_TRACE_MAX+1};rejected(&r);
    d=(ErrorReportDiagnostics){.trace_count=255};rejected(&r);
    d=(ErrorReportDiagnostics){.trace_count=1,.trace={(ErrorReportActivity)-1}};rejected(&r);
    d=(ErrorReportDiagnostics){.trace_count=1,.trace={ERROR_REPORT_ACTIVITY_COUNT}};rejected(&r);
    d=(ErrorReportDiagnostics){.trace_count=0,.trace={(ErrorReportActivity)-1}};
    assert(error_report_json(&r,out,sizeof(out),&n)); /* Unused trace slots are never serialized. */
    fputs("PASS: bounded JSON, UTF-8, consent-only data shape, enums, failure clearing and buffer canaries\n",stderr);
    fputs("PASS: schema 2 bounded snapshots, eight validity combinations, trace allowlist, schema 1 compatibility\n",stderr);
    return 0;
}
