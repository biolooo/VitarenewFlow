#pragma once
#include <3ds.h>
Result ds_prepare_launch(const char *rom);
Result ds_commit_launch(void);
Result ds_install_bootstrap(void (*progress)(float));
