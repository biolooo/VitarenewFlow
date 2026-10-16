#include "home_controls.h"
int home_footer_action(int x,int y,bool guide,bool hasGames){
    if(y<206||y>=234||x<8||x>=312)return HOME_NONE;
    if(x>=274)return HOME_GUIDE;
    if(x>=266)return HOME_NONE;
    if(guide)return HOME_BACK;
    return hasGames?HOME_OPEN:HOME_NONE;
}
int home_header_action(int x,int y){
    if(y<8||y>=44||x<8||x>=312)return HOME_NONE;
    if(x<44)return HOME_SETTINGS;
    if(x>=48&&x<76)return HOME_PREVIOUS;
    if(x>=244&&x<274)return HOME_NEXT;
    if(x>=278||(x>=78&&x<242))return HOME_CATEGORIES;
    return HOME_NONE;
}
unsigned home_category_target(unsigned current,unsigned count,bool left,bool right){
    if(!count)return 0;
    current%=count;
    return left==right?current:left?(current+count-1)%count:(current+1)%count;
}
