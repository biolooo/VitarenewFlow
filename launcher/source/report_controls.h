// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "report_transport.h"
typedef enum {REPORT_ACTION_NONE,REPORT_ACTION_CLOSE,REPORT_ACTION_CANCEL,REPORT_ACTION_EDIT,REPORT_ACTION_REVIEW,REPORT_ACTION_SEND} ReportAction;
ReportAction report_controls_action(ReportSendState state,bool configured,bool confirmed,bool back,bool primary,bool edit);
bool report_controls_hit(int x,int y);
bool report_controls_support_hit(int x,int y);
bool report_controls_fallback(bool available,bool unexpected,bool active_error);
