#include <common.h>

// NOP while loading from ND Box
void DECOMP_MM_Title_SetTrophyDPP(void)
{
  struct Instance* iVar1;
  struct Instance* iVar2;
  struct Title* title = OVR_230.titleObj;
  
  // if title object exists
  if (title != NULL) 
  {
	// instance named "title"
    iVar2 = title->i[3];
	
	// another instance named "title"
    iVar1 = title->i[2];
	
    if ((iVar2->idpp->unkb8[0] & 0x100) == 0) 
	{
	  // set Instance DrawPerPlayer,
	  // copy from one instance to the other
      iVar1->idpp->unkb8[0] = iVar1->idpp->unkb8[0] & (iVar2->idpp->unkb8[0] | 0xffffffbf);
      (iVar1->idpp->unkE4[0]) = (iVar2->idpp->unkE4[0]);
      (iVar1->idpp->unkE4[4]) = (iVar2->idpp->unkE4[4]);
      *(short*)(iVar1->idpp + 0xdc) = *(short*)(iVar2->idpp + 0xdc);
      *(short*)(iVar1->idpp + 0xde) = *(short*)(iVar2->idpp + 0xde);
    }
  }
}