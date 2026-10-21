// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#define SEARCH_QUERY_SIZE 33
#define SEARCH_TITLE_SIZE 192
void search_normalize(char *out,size_t capacity,const char *input);
/* Positive relevance or zero; inputs must be normalized. All words must match. */
int search_score(const char *title,const char *query);
/* Shared drawing/hit-test geometry: keyboard and action rows. */
char search_key_at(int x,int y,int numeric);
int search_edit(char query[SEARCH_QUERY_SIZE],char key);
const char *search_key_row(unsigned row,int numeric);
void search_suggest_add(char words[3][SEARCH_QUERY_SIZE],const char *query,const char *title);
int search_accept_suggestion(char query[SEARCH_QUERY_SIZE],const char *word);
