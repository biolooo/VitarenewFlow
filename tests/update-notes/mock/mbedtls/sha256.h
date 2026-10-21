#pragma once
#include <stddef.h>
#include <string.h>
typedef struct {int unused;} mbedtls_sha256_context;
static inline void mbedtls_sha256_init(mbedtls_sha256_context *s){s->unused=0;}
static inline void mbedtls_sha256_free(mbedtls_sha256_context *s){(void)s;}
static inline int mbedtls_sha256_starts_ret(mbedtls_sha256_context *s,int type){(void)s;(void)type;return 0;}
static inline int mbedtls_sha256_update_ret(mbedtls_sha256_context *s,const unsigned char *b,size_t n){(void)s;(void)b;(void)n;return 0;}
static inline int mbedtls_sha256_finish_ret(mbedtls_sha256_context *s,unsigned char out[32]){(void)s;memset(out,0,32);return 0;}
