#include <common.h>

int Player_Driving_LerpToForwards(struct Driver *d, int param_2, int param_3, int param_4)
{
  bool bVar1;
  uint uVar2;
  int iVar3;

  bVar1 = false;
  d->unk3CA = 0;
  if ((param_4 < 0) || ((param_4 == 0 && (param_2 < 0))))
  {
    bVar1 = true;
    param_2 = -param_2;
    param_3 = -param_3;
    param_4 = -param_4;
  }
  iVar3 = 0;

  if (d->set_0xF0_OnWallRub != 0xf0)
  {
    if (param_4 < param_2)
    {
      if (d->const_modelRotVelMax < param_2)
      {
        uVar2 = d->unk458 * 0xf;
      }
      else
      {
        uVar2 = d->unk458;
      }
      iVar3 = Player_Driving_LerpQuarterStrength(uVar2, param_2 - param_4, 0);
      iVar3 = -iVar3;
    }
    else
    {
      if (param_2 < param_4)
      {
        if (param_2 < 0)
        {
          iVar3 = Player_Driving_LerpQuarterStrength(d->unk459, param_4 -param_2, 0);
        }
        else
        {
          iVar3 = Player_Driving_LerpQuarterStrength(d->unk457, param_4 -param_2, 0);
          d->unk3CA = (short)param_4;
        }
      }
    }
  }

  // Interpolate rotation by speed
  iVar3 = InterpBySpeed(param_3, d->unk45a, iVar3);
  if (bVar1)
  {
    iVar3 = -iVar3;
  }
  return iVar3;
}