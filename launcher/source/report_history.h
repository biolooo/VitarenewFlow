// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "error_report.h"
typedef struct {ErrorReportActivity events[ERROR_REPORT_TRACE_MAX];uint8_t count;uint32_t generation;} ReportHistory;
void report_history_record(ReportHistory *history,ErrorReportActivity activity);
void report_history_snapshot(const ReportHistory *history,ErrorReportDiagnostics *snapshot);
