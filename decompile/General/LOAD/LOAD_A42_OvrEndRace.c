#include <common.h>

void OVR_Region1();

// DLL loaded = param_1 + 221
void DECOMP_LOAD_OvrEndRace(unsigned int param_1)
{
  // if new EndOfRace overlay needs to load
  if ((unsigned int)sdata->gGT->overlayIndex_EndOfRace != param_1)
  {
	  
#ifndef REBUILD_PC
    sdata->load_inProgress = 1;

	// EndOfRace overlay 221-225
    DECOMP_LOAD_AppendQueue(sdata->ptrBigfileCdPos_2,LT_RAW,(param_1 + 0xdd),&OVR_Region1,&DECOMP_LOAD_Callback_Overlay_Generic);
#endif

	// save ID, and reload next overlay (sector read invalidation)
	sdata->gGT->overlayIndex_EndOfRace = (char)param_1;
    sdata->gGT->overlayIndex_LOD = 0xff;
  }
  return;
}