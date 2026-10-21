from pathlib import Path
p=Path('outputs/3ds-library/source')
s=(p/'rom_backend.c').read_text(encoding='utf8')
fn=s[s.index('Result rom_preflight'):s.index('static Result command')]
test='''#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "launch_problem.h"
typedef int Result;
'''+fn+'''
int main(void){
 FILE *f=fopen("good.3dsx","wb");assert(f);fwrite("3DSX",1,4,f);fclose(f);
 f=fopen("game.n64","wb");assert(f);for(int i=0;i<192;i++)fputc(0,f);fclose(f);
 assert(rom_preflight("absent","game.n64")==-20);
 assert(rom_preflight(".","game.n64")==-21);
 assert(rom_preflight("game.n64","game.n64")==-21);
 assert(rom_preflight("good.3dsx","absent")==-22);
 assert(rom_preflight("good.3dsx","good.3dsx")==-22);
 assert(rom_preflight("good.3dsx","game.n64")==0);
 int expected[]={-1,-1,0,2,3,4,5,0};
 for(int i=0;i<SYSTEM_COUNT;i++){
  assert(launch_repair_component(i,-20)==expected[i]);
  assert(launch_repair_component(i,-21)==expected[i]);
  for(int e=-24;e<=0;e++)if(e!=-20&&e!=-21)assert(launch_repair_component(i,e)==-1);
 }
 assert(launch_repair_component(99,-20)==-1);
 puts("PASS: real ROM preflight and emulator recovery mapping");
}
'''
Path('work/launch-v01223/test.c').write_text(test,encoding='utf8')
assert 'launchFailed&&launchRepairComponent>=0&&!(keys&KEY_B)' in (p/'main.c').read_text(encoding='utf8')
assert '.launchRepair=launchFailed&&launchRepairComponent>=0' in (p/'main.c').read_text(encoding='utf8')
