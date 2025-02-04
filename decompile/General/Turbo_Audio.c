#include <common.h>

void DECOMP_Turbo_Audio(struct Driver* driver, int speed_cap) 
{
    u_int distortion;
    u_int volume;
	u_int extraFlags;

    // if turbo audio cooldown is not done
    if (driver->turbo_audioCooldown != 0) {
        return;
    }

    if (speed_cap > 0x7f) 
	{
        // max volume
        volume = 0xff << 0x10;

        // distort
        distortion = 0x6c << 8;

        // Make driver talk
        Voiceline_RequestPlay(0x10,data.characterIDs[driver->driverID],0x10);
		
		goto Skip;
    }
	
	if (speed_cap > 0x40) 
	{    
        // 3/4 volume
        volume = 0xc0 << 0x10;
		
        // no distort
        distortion = 0x80 << 8;
		
		goto Skip;
    }

    // half volume
    volume = 0x80 << 0x10;
	
    // distort
    distortion = 0x94 << 8;
	
Skip:

	// 50% L/R
	extraFlags = 0x80;

    // if echo is required
    if ((driver->actionsFlagSet & 0x10000) != 0) 
	{
        // add echo
        extraFlags |= 0x1000000;
    }

    // 0xD = Turbo Boost Sound
    // 0x80 = balance L/R
    OtherFX_Play_LowLevel(0xd,1, volume | distortion | 0x80);

    // turbo audio cooldown 0.24s
    driver->turbo_audioCooldown = 0xf0;
}