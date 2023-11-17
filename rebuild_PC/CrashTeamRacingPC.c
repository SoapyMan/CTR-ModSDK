#define _CRT_SECURE_NO_WARNINSG

#include "SDL_keycode.h"

#include "../rebuild_PS1/main.c"
#include "../decompile/General/zGlobal_DATA.c"
#include "../decompile/General/zGlobal_SDATA.c"

static int btnFlags = 0;

int WIN_GetPressUp()
{
	return btnFlags & 0x1;
}

int WIN_GetPressDown()
{
	return btnFlags & 0x2;
}

int WIN_GetPressLeft()
{
	return btnFlags & 0x4;
}

int WIN_GetPressRight()
{
	return btnFlags & 0x8;
}

int WIN_GetPressCross()
{
	return btnFlags & 0x10;
}

int WIN_GetPressTriangle()
{
	return btnFlags & 0x20;
}

void PsyXKeyboardHandler(int key, char down)
{
	// here goes your SDL_SCANCODE_* debug keys
	if(key == SDL_SCANCODE_UP)
		btnFlags = down ? (btnFlags | 0x1) : (btnFlags & ~0x1);
	if (key == SDL_SCANCODE_DOWN)
		btnFlags = down ? (btnFlags | 0x2) : (btnFlags & ~0x2);
	if (key == SDL_SCANCODE_LEFT)
		btnFlags = down ? (btnFlags | 0x4) : (btnFlags & ~0x4);
	if (key == SDL_SCANCODE_RIGHT)
		btnFlags = down ? (btnFlags | 0x8) : (btnFlags & ~0x8);
	if (key == SDL_SCANCODE_X)
		btnFlags = down ? (btnFlags | 0x10) : (btnFlags & ~0x10);
	if (key == SDL_SCANCODE_C)
		btnFlags = down ? (btnFlags | 0x20) : (btnFlags & ~0x20);
}

int main()
{
	PsyX_Initialise("CTRPC", 800, 600, 0);
	PsyX_CDFS_Init("ctr-u.bin", 0, 0);

	g_dbg_gameDebugKeys = PsyXKeyboardHandler;

	// set to 30 FPS VSync
	PsyX_SetSwapInterval(2);
	PsyX_EnableSwapInterval(1);

	return DECOMP_main();
}