#include <common.h>

void DECOMP_CTR_CycleTex_2p3p4pWumpaHUD(u_long *tileViewOT, u_long *decalMpOT, int quarterBuffer)
{
  *decalMpOT = *tileViewOT
  *tileViewOT = (u_long)(decalMpOT + quarterBuffer - 1) & 0xffffff;
}