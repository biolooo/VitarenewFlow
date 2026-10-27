// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#include <stddef.h>
#define DS_ROOT_MAX 8
#define DS_PATH_MAX 512
#define DS_FOLDER_MAX 96
typedef struct {unsigned count;char paths[DS_ROOT_MAX][DS_PATH_MAX];} DsRoots;
typedef struct {unsigned directories,missing,files,entries;bool limited,cancelled;} DsScan;
typedef struct {char path[DS_PATH_MAX];char names[DS_FOLDER_MAX][256];unsigned count,selected;bool limited,unreadable;} DsBrowser;
typedef bool (*DsVisit)(const char *,void *);
typedef bool (*DsContinue)(void *);
bool ds_root_normalize(char *out,size_t capacity,const char *path);
bool ds_roots_add(DsRoots *roots,const char *path);
void ds_roots_load(DsRoots *roots,const char *config);
bool ds_roots_save(const DsRoots *roots,const char *config);
void ds_library_scan(const DsRoots *roots,DsScan *result,DsVisit visit,DsContinue proceed,void *user);
void ds_browser_read(DsBrowser *browser);
bool ds_browser_enter(DsBrowser *browser,unsigned index);
bool ds_browser_parent(DsBrowser *browser);
int ds_empty_hit(int x,int y);
