#include <common.h>

void DECOMP_GameInit_PrimMem(struct GameTracker* gGT)
{
	int size;
	int levelID = gGT->levelID;
	
	size = 0x1b800;
	goto EndFunc;
	
	// adv garage
	if(levelID == 0x28)
	{
		size = 0x1b800;
		goto EndFunc;
	}
	
	// main menu
	if(levelID == 0x27)
	{
		size = 0x17c00;
		goto EndFunc;
	}
	
	if(gGT->numPlyrCurrGame == 1)
	{
		// any% end, 101% end, credits
		if(levelID >= 42)
		{
			size = 0x17c00;
			goto EndFunc;
		}
		
		// intro cutscene
		if(levelID >= 30)
		{
			size = 0x1e000;
			goto EndFunc;
		}
		
		// adv hub
		if(levelID >= 25)
		{
			size = 0x1c000;
			goto EndFunc;
		}
		
		// ordinary tracks

		// all are 0x67 or 0x5F, adv hub was 0x5F too
		size = data.primMem_SizePerLEV_1P[levelID] << 10;
		goto EndFunc;
	}
	
	if(gGT->numPlyrCurrGame == 2)
	{
		// assume only levID 0-24
		size = data.primMem_SizePerLEV_2P[levelID] << 10;
		goto EndFunc;
	}
	
	// 3P 4P
	// assume only levID 0-24
	size = data.primMem_SizePerLEV_4P[levelID] << 10;
	
EndFunc:
	MainDB_PrimMem(&gGT->db[0].primMem, size);
	MainDB_PrimMem(&gGT->db[1].primMem, size);
}

void DECOMP_GameInit_OTMem(struct GameTracker* gGT)
{
	int size;
	int levelID = gGT->levelID;
	
	size = 0x2c00;
	goto EndFunc;
	
	// cutscenes, main menu, garage, ND Box, 
	// any% end, 101% end, credits
	if(levelID >= 30)
	{
		size = 0x2000;
		goto EndFunc;
	}
	
	// Adv Hub
	if(levelID >= 25)
	{
		size = 0x2c00;
		goto EndFunc;
	}
	
	// battle maps
	if(levelID >= 18)
	{
		size = 0x8000;
		goto EndFunc;
	}
	
	// 1P/2P mode
	if(gGT->numPlyrCurrGame < 3)
	{
		size = 0x2000;
		goto EndFunc;
	}
	
	// 3P/4P mode
	size = 0x3000;
	
EndFunc:
	MainDB_OTMem(&gGT->db[0].otMem, size);
	MainDB_OTMem(&gGT->db[1].otMem, size);
	
	// 0x1000 per player, plus 0x18 for linking
	size = ((gGT->numPlyrCurrGame) << 0xC) | 0x18;
	gGT->ot_camera110_UI[0] = MEMPACK_AllocMem(size); // "ot1"
	gGT->ot_camera110_UI[1] = MEMPACK_AllocMem(size); // "ot2"
}