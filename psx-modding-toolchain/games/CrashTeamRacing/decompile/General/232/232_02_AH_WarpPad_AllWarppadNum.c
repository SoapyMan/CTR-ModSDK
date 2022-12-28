#include <common.h>

void AH_WarpPad_SetNumModelData(struct Instance* inst, struct ModelHeader* mh);

void DECOMP_AH_WarpPad_AllWarppadNum()
{
	struct WarpPad* wp;
	struct ModelHeader* mh;
	struct Instance* inst;
	
	struct Thread* t =
		sdata->gGT->threadBuckets[WARPPAD].thread;
	
	wp = t->object;
	
	for(t; t != 0; t = t->siblingThread)
	{
		inst = wp->inst[2];
		mh = &inst->model->headers[0];
		AH_WarpPad_SetNumModelData(inst, &mh[wp->digit1s-1]);
		
		// need to fix
		if(wp->inst[3] != 0)
		{
			inst = wp->inst[3];
			mh = &inst->model->headers[0];
			AH_WarpPad_SetNumModelData(inst, &mh[1-1]);
		}
	}
}

void AH_WarpPad_SetNumModelData(struct Instance* inst, struct ModelHeader* mh)
{
	// it's 4am, I dont care, I'm going to bed - Niko
	inst->idpp[0].ptrCommandList = 	mh->ptrCommandList;
	inst->idpp[0].ptrColorLayout = 	mh->ptrColors;
	inst->idpp[0].ptrTexLayout = 	mh->ptrTexLayout;
	inst->idpp[0].unkc0[0] = 		mh->ptrVertexData;
}