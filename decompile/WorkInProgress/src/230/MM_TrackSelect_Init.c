#include <common.h>

void DECOMP_MM_TrackSelect_Init(void)
{
  struct MainMenu_LevelRow *selectMenu;
  short numTracks;

  // lap selection menu is closed by default
  OVR_230.trackSel_boolOpenLapBox = false;
  OVR_230.trackSel_transitionState = 0;

  // set track index to the index selected in track selection menu, starts at 0 for both Arcade and Battle
  OVR_230.menubox_trackSelect.rowSelected = sdata->trackSelIndex;
  
  // 12 frames when moving between selection
  OVR_230.trackSel_transitionFrames = 12;
  
// Set menu and num of tracks based on game mode
  if ((sdata->gGT->gameMode1 & BATTLE_MODE) != 0)
  {
    selectMenu = &OVR_230.battleTracks[0];
    numTracks = 7;
  }
  else
  {
    selectMenu = &OVR_230.arcadeTracks[0];
    numTracks = 18;
  }

  // If you scroll past the max number of tracks, go back to the first track
  if (numTracks <= sdata->trackSelIndex)
  {
    OVR_230.menubox_trackSelect.rowSelected = 0;
  }

  // Loop through all tracks until an unlocked track is found
  while (!MM_TrackSelect_boolTrackOpen(selectMenu[OVR_230.menubox_trackSelect.rowSelected << 4]))
  {
    OVR_230.menubox_trackSelect.rowSelected++;

    // If track index goes too high, reset to zero
    if (numTracks <= OVR_230.menubox_trackSelect.rowSelected)
    {
      OVR_230.menubox_trackSelect.rowSelected = 0;
    }
  }

  OVR_230.trackSel_currTrack = OVR_230.menubox_trackSelect.rowSelected;

  MM_TrackSelect_Video_SetDefaults();
}
