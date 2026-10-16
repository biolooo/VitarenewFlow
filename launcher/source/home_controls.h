#pragma once
#include <stdbool.h>
enum HomeAction {HOME_NONE,HOME_OPEN,HOME_GUIDE,HOME_BACK,HOME_SETTINGS,HOME_PREVIOUS,HOME_NEXT,HOME_CATEGORIES};
int home_footer_action(int x,int y,bool guide,bool hasGames);
int home_header_action(int x,int y);
unsigned home_category_target(unsigned current,unsigned count,bool left,bool right);
