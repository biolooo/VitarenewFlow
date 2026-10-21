#include "copy_result.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void check(const char *data,size_t size,CopyResult expected){
 FILE *f=fopen("/tmp/vf-copy-result-test.txt","wb");assert(f);assert(fwrite(data,1,size,f)==size);assert(!fclose(f));
 assert(copy_result_take("/tmp/vf-copy-result-test.txt")==expected);
 if(size<40)assert(copy_result_take("/tmp/vf-copy-result-test.txt")==COPY_NONE);
}
int main(void){
 assert(copy_result_take("/tmp/vf-no-such-receipt")==COPY_NONE);
 check("VF1 DS\n",7,COPY_DS_READY);check("VF1 CIA\n",8,COPY_CIA_READY);
 check("VF1 DS\r\n",8,COPY_DS_READY);check("VF1 CIA\r\n",9,COPY_CIA_READY);
 check("VF1 STARTED\n",12,COPY_INCOMPLETE);check("",0,COPY_INCOMPLETE);
 check("VF1 DS",6,COPY_INCOMPLETE);check("VF1 DS\nJUNK",11,COPY_INCOMPLETE);
 check("VF1 DS\n\0",8,COPY_INCOMPLETE);check("VF1 /../../boot.3dsx\n",21,COPY_INCOMPLETE);
 check("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",40,COPY_INCOMPLETE);
 puts("PASS actual native receipt reader: DS/CIA, CRLF, cancellation, missing/truncated/oversize/malformed/NUL receipts and one-time acknowledgement");
}
