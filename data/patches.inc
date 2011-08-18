/*
 *  Sound effect patches
 *  Copyright (C) 2011  Steve Maddison
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

typedef enum {
	// Instruments
	PATCH_LEAD1 = 0,
	PATCH_LEAD2,
	PATCH_BASS,
	PATCH_HI_HAT,
	// Sound FX
	PATCH_TICK,
	PATCH_SHOOT,
	PATCH_WIN1,
	PATCH_WIN2,
	PATCH_LOSE
} patch_t;

// Instruments
const char patch_lead1[] PROGMEM ={ 
	0,PC_WAVE,1,
	0,PC_ENV_VOL,0xff,
	0,PC_ENV_SPEED,-10,
	0,PC_TREMOLO_LEVEL,0x60,     
	0,PC_TREMOLO_RATE,50,
	0,PATCH_END
};

const char patch_lead2[] PROGMEM ={ 
	0,PC_WAVE,3,
	0,PC_ENV_VOL,0xff,
	0,PC_ENV_SPEED,-10,
	0,PC_TREMOLO_LEVEL,0x60,     
	0,PC_TREMOLO_RATE,50,
	0,PATCH_END
};

const char patch_bass[] PROGMEM ={ 
	0,PC_WAVE,4,
	0,PC_ENV_VOL,0xa0,
	0,PC_NOTE_UP,12,
	0,PC_ENV_SPEED,0,
	0,PATCH_END
};

const char patch_hi_hat[] PROGMEM ={ 
	0,PC_NOISE_PARAMS,1,
	0,PC_ENV_VOL,0x90,
	0,PC_ENV_SPEED,-30, 
	8,PC_NOTE_CUT,0,
	0,PATCH_END
};

// Sound FX
const char patch_tick[] PROGMEM = {
	0,PC_NOISE_PARAMS,1,
	0,PC_ENV_VOL,0x90,
	0,PC_ENV_SPEED,-30, 
	3,PC_NOTE_CUT,0,
	0,PATCH_END
};

const char patch_shoot[] PROGMEM = {
	0,PC_ENV_SPEED,-16,
	0,PC_ENV_VOL,0xf0,
	0,PC_WAVE,0,
	0,PC_TREMOLO_LEVEL,0xf0,     
	0,PC_TREMOLO_RATE,60,
	0,PC_PITCH,70,
	2,PC_NOTE_UP,2,
	2,PC_NOTE_UP,4,
	2,PC_NOTE_UP,6,
	2,PC_NOTE_UP,8,
	2,PC_NOTE_UP,2,
	2,PC_NOTE_UP,2,
	2,PC_NOTE_DOWN,4,
	20,PC_NOTE_CUT,0,
	0,PATCH_END
};

const char patch_win1[] PROGMEM = {
	0,PC_WAVE,2,
	0,PC_PITCH,60,
	0,PC_ENV_VOL,0xff,

	16,PC_NOTE_UP,4,
	18,PC_NOTE_UP,4,

	20,PC_NOTE_UP,8,
	0,PC_ENV_SPEED,-12,
	24,PC_ENV_VOL,0xff,
	10,PC_ENV_VOL,0xff,
	10,PC_ENV_VOL,0xff,
	0,PC_ENV_SPEED,-1,
	90,PC_NOTE_CUT,0,
	0,PATCH_END
};

const char patch_win2[] PROGMEM = {
	0,PC_WAVE,4,
	0,PC_PITCH,70,
	0,PC_ENV_VOL,0xff,

	16,PC_NOTE_UP,4,
	18,PC_NOTE_UP,4,

	20,PC_NOTE_UP,8,
	0,PC_ENV_SPEED,-12,
	24,PC_ENV_VOL,0xff,
	10,PC_ENV_VOL,0xff,
	10,PC_ENV_VOL,0xff,
	0,PC_ENV_SPEED,-1,
	90,PC_NOTE_CUT,0,
	0,PATCH_END
};

const char patch_lose[] PROGMEM = {
	0,PC_WAVE,6,
	0,PC_PITCH,54,
	0,PC_ENV_VOL,0xff,
	16,PC_NOTE_DOWN,1,
	16,PC_NOTE_DOWN,1,
	20,PC_NOTE_DOWN,1,
	0,PC_TREMOLO_LEVEL,0x40,
	0,PC_TREMOLO_RATE,20,
	0,PC_ENV_VOL,0xff,
	0,PC_ENV_SPEED,-1,
	120,PC_NOTE_CUT,0,
	0,PATCH_END
};

const struct PatchStruct patches[] PROGMEM = {
	// Instruments
	{0,NULL,patch_lead1,0,0},
	{0,NULL,patch_lead2,0,0},
	{0,NULL,patch_bass,0,0},
	{1,NULL,patch_hi_hat,0,0},
	// SOund FX
	{1,NULL,patch_tick,0,0},
	{0,NULL,patch_shoot,0,0},
	{0,NULL,patch_win1,0,0},
	{0,NULL,patch_win2,0,0},
	{0,NULL,patch_lose,0,0}
};

