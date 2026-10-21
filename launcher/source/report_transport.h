// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#include "error_report.h"
typedef enum {REPORT_IDLE,REPORT_SENDING,REPORT_SENT,REPORT_FAILED,REPORT_CANCELLED} ReportSendState;
/* Public intake only; no report listing or client secrets. Approved 2026-10-21. */
#ifndef FLOW_REPORT_ENDPOINT
#define FLOW_REPORT_ENDPOINT "https://vitarenew-lager.tail6ccd16.ts.net:10000/flow-reports/v1/reports"
#endif
bool report_transport_configured(void);
bool report_transport_start(const ErrorReport *report, bool consent);
ReportSendState report_transport_state(void);
void report_transport_cancel(void);
void report_transport_reset(void);
void report_transport_stop(void);
