#define _CRT_SECURE_NO_WARNINSG
#define REBUILD_PC
#include "Library/LIBCD.H"
#include "Library/LIBGPU.H"
#include "Library/LIBSPU.H"
#include "Library/KERNEL.H"
#define u_char unsigned char
#define u_short unsigned short
#define u_int unsigned int
#define u_long unsigned int
#define _Static_assert(x) ;
#define __attribute__(x) ;
#define BUILD 926
#define RECT PSX_RECT

// Missing from PsyCross
char* CdComstr(u_char com) {}
CdlCB CdReadCallback(CdlCB func) {}
CdlCB CdSyncCallback(CdlCB func) {}
u_long* BreakDraw(void) {}
SpuTransferCallbackProc SpuSetTransferCallback(SpuTransferCallbackProc func) {}
long SpuReadDecodedData(SpuDecodedData* d_data, long flag) {}
long SpuSetIRQ(long on_off) {}
unsigned long SpuSetIRQAddr(unsigned long x) {}
SpuIRQCallbackProc SpuSetIRQCallback(SpuIRQCallbackProc x) {}
void SpuSetCommonCDMix(long cd_mix) {}
extern void SpuSetCommonCDVolume(short cd_left, short cd_right) {}
void SpuSetCommonCDReverb(long cd_reverb) {}

#include "../rebuild_PS1/main.c"
#include "../decompile/General/zGlobal_DATA.c"
#include "../decompile/General/zGlobal_SDATA.c"

int main()
{
	return DECOMP_main();
}