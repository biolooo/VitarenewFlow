#include "device_settings.h"
#include <limits.h>
static const u32 luminanceLevels[]={20,43,73,95,117};
bool device_brightness_read(int *level,u32 *luminance){
    if(R_FAILED(gspLcdInit()))return false;
    u32 value=0;
    Result r=GSPLCD_GetBrightness(GSP_SCREEN_TOP,&value);
    gspLcdExit();
    if(R_FAILED(r))return false;
    unsigned delta=UINT_MAX;
    for(int i=0;i<5;++i){unsigned d=value>luminanceLevels[i]?value-luminanceLevels[i]:luminanceLevels[i]-value;
    if(d<delta){delta=d;
    *level=i;}}
    *luminance=value;
    return true;
}
Result device_brightness_set(unsigned level,u32 *before,u32 *after){
    if(level>=5)return (Result)-50;
    *before=*after=0;
    Result r=gspLcdInit();
    if(R_FAILED(r))return r;
    bool readable=R_SUCCEEDED(GSPLCD_GetBrightness(GSP_SCREEN_TOP,before));
    /* SetBrightness is SetLuminanceLevel: its value is a PRESET 1..5.
       v0.7.0 incorrectly supplied raw luminances (20..128) here. */
    r=GSPLCD_SetBrightness(GSPLCD_SCREEN_BOTH,level+1);
    if(R_SUCCEEDED(r))GSPLCD_GetBrightness(GSP_SCREEN_TOP,after);
    if(R_FAILED(r)||(readable&&*before==*after)){
        r=GSPLCD_SetBrightnessRaw(GSPLCD_SCREEN_BOTH,luminanceLevels[level]);
        if(R_SUCCEEDED(r))GSPLCD_GetBrightness(GSP_SCREEN_TOP,after);
    }
    gspLcdExit();
    return r;
}
