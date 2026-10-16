#pragma once
#include <stdbool.h>
const char *tr(const char *text);
bool l10n_english(void);
void l10n_load(bool englishDefault);
bool l10n_save(bool english);
