// SPDX-License-Identifier: GPL-3.0-only
#include "component_catalog.h"
static const ComponentFile files0[]={
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/gba-mgba-library.3dsx","sdmc:/3ds/mgba-library/mgba-library.3dsx","2b898bf9be8eb4cb334c34a8bb1eed3b97c0e5375166f429b34aa3411707fa96",1525236},
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/gba-mgba-library.smdh","sdmc:/3ds/mgba-library/mgba-library.smdh","c62f92ec821be9810785930142a50e190aca179652d183fa32c5ee2f0f1d781b",14016},
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/gba-LICENSE","sdmc:/3ds/mgba-library/LICENSE","fab3dd6bdab226f1c08630b1dd917e11fcb4ec5e1e020e2c16f83a0a13863e85",16726},
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/gba-ADAPTER.txt","sdmc:/3ds/mgba-library/ADAPTER.txt","9856a3be041edae5c5140780be1cd989e2ad037dc58edb8e8fa8d71bae3011bc",1494},
};
static const ComponentFile files1[]={
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/ds-sdcard.nds","sdmc:/_nds/ntr-forwarder/sdcard.nds","2a389fde68fe1120b1e60e18e3a42c86f426bdb06eb85ec8da36fe89bef7f5e9",1693184},
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/ds-nds-bootstrap-release.nds","sdmc:/_nds/nds-bootstrap-release.nds","39a3a5e62ea8d1f700c3735199424b1acdd0ccfcfcd31d61bc69984cb2e10b4c",1712128},
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/ds-bootstrap.cia","sdmc:/3ds/3ds-library/components/ds-bootstrap.cia","da954fc7074f6ef8fb17d2dd8e7d387baf64b08affbb2d13c2016a62edf2a5bd",206592},
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/ds-LICENSE.txt","sdmc:/3ds/3ds-library/components/ds-LICENSE.txt","480d2dc40a02a24fa9af3b14af745477bb4a2a5c09ed545e1e9a71dcfad38883",72735},
};
static const ComponentFile files2[]={
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/n64-n64-library.3dsx","sdmc:/3ds/n64-library/n64-library.3dsx","9abf44c3564ed401f267e3aabf457ac7acf67ede1b48ecb30bb0efee3e2be845",1378500},
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/n64-n64-library.smdh","sdmc:/3ds/n64-library/n64-library.smdh","b7cafea9f64d31dcafd87d13f5e3b59e7feb1373e8c17d54235da53f4d7f93d9",14016},
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/n64-UPSTREAM-copying.txt","sdmc:/3ds/n64-library/UPSTREAM-copying.txt","67f82e045cf7acfef853ea0f426575a8359161a0a325e19f02b529a87c4b6c34",15131},
 {"https://biolooo.github.io/VitarenewFlow/updates/components-1/n64-ADAPTER.txt","sdmc:/3ds/n64-library/ADAPTER.txt","c43fecf9594f2ac92f18350b615630a2410767a9adeefd4478bc7e990a1774c1",304},
};
static const ComponentFile desktop_files[]={
 {"https://biolooo.github.io/VitarenewFlow/updates/desktop-1/VitarenewFlow-Desktop.exe","sdmc:/VitarenewFlow Desktop.exe","3881305b2372d5a96021959a64046be82b3a3778e4ebb063dfb7597470a949be",126976},
 {"https://biolooo.github.io/VitarenewFlow/updates/desktop-1/LIESMICH.txt","sdmc:/Library-PC/LIESMICH.txt","1fc507e3d6f3cd70317a9455638d6187f80a4a836bd0ae4e7d6d22dbf14fb0d4",1297},
 {"https://biolooo.github.io/VitarenewFlow/updates/desktop-1/LICENSE.txt","sdmc:/Library-PC/LICENSE.txt","3972dc9744f6499f0f9b2dbf76696f2ae7ad8af9b23dde66d6af86c9dfb36986",35149},
 {"https://biolooo.github.io/VitarenewFlow/updates/desktop-1/VitarenewFlow-Desktop-source.zip","sdmc:/Library-PC/VitarenewFlow-Desktop-source.zip","e530c83a0f4e15132a4cf9595f671176f3691b3fe9733ce56f0224f123f183f8",53042},
};
static const ComponentFile retro_files3[]={
 {"https://biolooo.github.io/VitarenewFlow/updates/retro-1/snes-snes-library.3dsx","sdmc:/3ds/snes-library/snes-library.3dsx","3b16cee18393f4d5ea86c533b74cf591b2e844c0456f347809c80a484948f336",2492796},
 {"https://biolooo.github.io/VitarenewFlow/updates/retro-1/snes-snes-library.smdh","sdmc:/3ds/snes-library/snes-library.smdh","e6e12a5c42924536563714dd63e4140bd8a95e2bb6bb94a703994345116abcaf",14016},
 {"https://biolooo.github.io/VitarenewFlow/updates/retro-1/snes-NOTICES.txt","sdmc:/3ds/snes-library/NOTICES.txt","4e5f2d929b368e275215bb0c0f3e0ea8a9f5c045ca8dabff540a8c790d8121c0",1785},
};
static const ComponentFile retro_files4[]={
 {"https://biolooo.github.io/VitarenewFlow/updates/retro-1/nes-nes-library.3dsx","sdmc:/3ds/nes-library/nes-library.3dsx","a04d8817fab580668c5d2ba12c979e7cf6784b524b93fbaed40e4a84062b17e0",1097876},
 {"https://biolooo.github.io/VitarenewFlow/updates/retro-1/nes-nes-library.smdh","sdmc:/3ds/nes-library/nes-library.smdh","396b61b0878ff9704463de7cc5a25fcefa486227b203c8ddda86ade6b7d9408f",14016},
 {"https://biolooo.github.io/VitarenewFlow/updates/retro-1/nes-NOTICES.txt","sdmc:/3ds/nes-library/NOTICES.txt","f049881c5e78bbc9752fc3a0320bcb0549ea37afd547c07a24d3e21cc1939dd9",8390},
};
static const ComponentFile retro_files5[]={
 {"https://biolooo.github.io/VitarenewFlow/updates/retro-1/vb-vb-library.3dsx","sdmc:/3ds/vb-library/vb-library.3dsx","86a6b1853fbd5541dc9a17dc38be07221a322052c91ae8c8e601c9b7b78033d6",681964},
 {"https://biolooo.github.io/VitarenewFlow/updates/retro-1/vb-vb-library.smdh","sdmc:/3ds/vb-library/vb-library.smdh","b2a3f7937f33903e67c2468bc13a94c54b12fdba4e612229d6bb7f848479cc69",14016},
 {"https://biolooo.github.io/VitarenewFlow/updates/retro-1/vb-NOTICES.txt","sdmc:/3ds/vb-library/NOTICES.txt","cb757844940bc4a181bb3aef264e1ebd9e42929df3666d41ec899ef9e61dbd4e",9576},
};
static const ComponentDef definitions[]={
 {"Game Boy","0.10.5 / Flow 3",files0,4},
 {"Nintendo DS","SD-30 / 2.17.0",files1,4},
 {"Nintendo 64","1.0 / Flow 0.6.1",files2,4},
 {"Super Nintendo","1.61 / Flow",retro_files3,3},
 {"Nintendo NES","1.02 / Flow 0.7.0",retro_files4,3},
 {"Virtual Boy","1.3.4 / Flow",retro_files5,3},
 {"VitarenewFlow Desktop","Windows · 0.4.1",desktop_files,4},
};
const ComponentDef *component_definition(unsigned index){return index<COMPONENT_COUNT?&definitions[index]:0;}
