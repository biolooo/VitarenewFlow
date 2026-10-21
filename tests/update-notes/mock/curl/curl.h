#pragma once
#include <stddef.h>
typedef struct TestCurl CURL;
typedef int CURLcode;typedef long long curl_off_t;
struct curl_blob {void *data;size_t len;unsigned flags;};
#define CURLE_OK 0
#define CURLE_WRITE_ERROR 23
#define CURLE_PEER_FAILED_VERIFICATION 60
#define CURL_GLOBAL_DEFAULT 0
#define CURL_BLOB_NOCOPY 0
#define CURLINFO_RESPONSE_CODE 1
enum {CURLOPT_URL,CURLOPT_CAINFO_BLOB,CURLOPT_SSL_VERIFYPEER,CURLOPT_SSL_VERIFYHOST,
      CURLOPT_PROTOCOLS_STR,CURLOPT_FOLLOWLOCATION,CURLOPT_PROXY,CURLOPT_NOSIGNAL,
      CURLOPT_CONNECTTIMEOUT,CURLOPT_TIMEOUT,CURLOPT_LOW_SPEED_LIMIT,CURLOPT_LOW_SPEED_TIME,
      CURLOPT_USERAGENT,CURLOPT_WRITEFUNCTION,CURLOPT_WRITEDATA,CURLOPT_NOPROGRESS,
      CURLOPT_XFERINFOFUNCTION};
int curl_global_init(long);void curl_global_cleanup(void);
CURL *curl_easy_init(void);void curl_easy_cleanup(CURL*);
int curl_easy_setopt(CURL*,int,...);int curl_easy_perform(CURL*);int curl_easy_getinfo(CURL*,int,long*);
