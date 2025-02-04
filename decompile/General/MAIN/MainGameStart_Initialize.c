#include <common.h>

void MainGameStart_Initialize(struct GameTracker* gGT, char boolStopAudio)
{
  u_int gameModeFlag = gGT->gameMode1 & ~(END_OF_RACE);  // Remove end-of-race flag

  // DotLights_AudioAndVideo wont execute
  // if (gGT & 0x20102000 == 0), but if it did execute,
  // traffic lights would instantly hit green in cutscene
  // and main menu, while starting from top-position (0xf00)
  // when spawning in the adventure arena

  // if you're not in cutscene and not in main menu
  if ((gGT->gameMode1 & (GAME_CUTSCENE | MAIN_MENU)) == 0)
  {
	// traffic light countdown
    gGT->trafficLightsTimer = 0xf00;

	// fly-in camera animation
    gameModeFlag |= START_OF_RACE;
  }
  // if you are:
  else
  {
	// disable traffic lights
    gGT->trafficLightsTimer = 0;

	// no fly-in camera
    gameModeFlag &= ~(START_OF_RACE);
  }

  // save new game mode
  gGT->gameMode1 = gameModeFlag;

  // this never happens in normal gameplay
  if (boolStopAudio == 0) {

    Music_Stop();

    // stops menu sounds
    // keep backup,
    // keep music (no music to stop),
    // stop all fx
      howl_StopAudio(0,0,1);
  }

  VehInit_TeleportAll(gGT,2);
}
