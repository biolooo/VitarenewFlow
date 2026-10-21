"""Compile production wrapping/paging with production font metrics; no GPU mock."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
font = (root / "launcher/source/ui_font.c").read_text()
ui = (root / "launcher/source/ui.c").read_text()
font_functions = font[font.index("static const UiFace *face"):font.index("bool ui_font_init")]
wrap = ui[ui.index("static bool wrap_next"):ui.index("static void ellipsis")]
paging = ui[ui.index("unsigned ui_update_notes_pages"):ui.index("static void chevron")]
render_capture = r'''
static const uint32_t ink=0;
static unsigned rendered_letters;
static void text(float x,float y,unsigned size,uint32_t color,const char *body){
 (void)color;assert(x==22&&y>=88&&y<=158&&size==11);
 for(const char *c=body;*c;c++)if(*c=='W')rendered_letters++;
}
'''
harness = r'''
int main(void){
 UpdateNotes notes={.count=1};
 for(unsigned i=0;i<9;i++)strcat(notes.de[0],i?" WWWWWWWWWWWW":"WWWWWWWWWWWW");
 strcpy(notes.en[0],"Short English text");
 unsigned pages=ui_update_notes_pages(&notes,false);
 assert(pages>=2); /* Valid 116-byte words must never disappear behind a six-line cutoff. */
 assert(ui_update_notes_pages(&notes,true)==1);
 rendered_letters=0;
 for(unsigned p=0;p<pages;p++){
  update_notes_page_draw(update_notes_page_text(&notes,false,p));
 }
 assert(rendered_letters==108); /* Every letter reachable exactly once across subpages. */
 strcpy(notes.de[1],notes.de[0]);strcpy(notes.en[1],"Second text");notes.count=2;
 assert(ui_update_notes_pages(&notes,false)==2*pages);
 assert(ui_update_notes_pages(&notes,true)==2);
 assert(!ui_update_notes_pages(NULL,false));
 puts("PASS: production font metrics/wrapping, long valid bullets span pages without loss, DE/EN page counts");
 return 0;
}
'''
source = ('#include <assert.h>\n#include <stdbool.h>\n#include <stdint.h>\n#include <string.h>\n#include <stdio.h>\n'
          '#include "update_format.h"\n#include "ui_font_data.h"\n' + font_functions + wrap + render_capture + paging + harness)
with tempfile.TemporaryDirectory(prefix="flow-note-layout-") as directory:
    path = Path(directory)
    (path / "test.c").write_text(source)
    subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-fsanitize=undefined",
                    "-I" + str(root / "launcher/source"), str(path / "test.c"), "-o", str(path / "test")], check=True)
    subprocess.run([str(path / "test")], check=True)
