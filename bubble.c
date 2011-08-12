/*
 *  A bubbly puzzle game for the Uzebox
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

#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <uzebox.h>

typedef enum {
	C_BLANK = 0,
	C_RED,
	C_ORANGE,
	C_YELLOW,
	C_GREEN,
	C_BLUE,
	C_PURPLE,
	C_BLACK,
	C_COUNT
} colour_t;

// Number of first bubble tile in the set
// (not literally, as first row is for blanks).
#define BUBBLE_FIRST_TILE			0
#define BUBBLE_FIRST_COLOUR_TILE	11
// Background tile for play field
#define BUBBLE_FIELD_TILE			1
// Offsets for bubbles on even-numbered rows
#define BUBBLE_EVEN_L				0
#define BUBBLE_EVEN_R_SPLIT			1
#define BUBBLE_EVEN_R_WHOLE			9
// Offsets for bubbles on odd-numbered rows
#define BUBBLE_ODD_MIDDLE			10
#define BUBBLE_ODD_L				11
#define BUBBLE_ODD_R				12
// Number of bubble tiles of each colour
#define BUBBLES_PER_COLOUR			13
#define BUBBLE_SLIVER_L				9
#define BUBBLE_SLIVER_R				10

#define FIELD_OFFSET_X		2
#define FIELD_OFFSET_Y		2

#include "data/bg.inc"
#include "data/sprites.inc"

#define PLAYERS				2
#define FIELD_BUBBLES_H		8
#define FIELD_BUBBLES_V		11
#define FIELD_TILES_H		12
#define FIELD_TILES_V		11
#define NUM_BUBBLES			83

// Pre-calcutated co-ordinates of arrow parts
#define ANGLES 30
const char arrow_x[ANGLES] PROGMEM = {
	0, 2, 3, 5, 6, 8, 10, 11, 13, 14, 16, 17, 18, 20, 21, 22,
	23, 24, 25, 26, 27, 28, 28, 29, 29, 30, 30, 31, 31, 31 };
const char arrow_y[ANGLES] PROGMEM = {
	31, 31, 31, 31, 30, 30, 29, 29, 28, 28, 27, 26, 25, 24,
	23, 22, 21, 20, 18, 17, 15, 14, 13, 11, 10, 8, 6, 5, 3, 2 };
const char ring_x[ANGLES] PROGMEM = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 14, 15,
	16, 16, 17, 17, 18, 18, 19, 19, 19, 20, 20, 20, 20 };
const char ring_y[ANGLES] PROGMEM = {
	20, 20, 20, 20, 20, 19, 19, 19, 18, 18, 17, 17, 16, 16,
	15, 14, 13, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
const char rivet_x[ANGLES] PROGMEM = {
	0, 1, 1, 2, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 9, 10, 10, 11,
	11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 14, 14 };
const char rivet_y[ANGLES] PROGMEM = {
	14, 14, 14, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11,
	10, 10, 9, 9, 8, 8, 7, 6, 6, 5, 4, 4, 3, 2, 1, 1 };


// Globals
unsigned char bubbles[PLAYERS][NUM_BUBBLES];
unsigned char current[PLAYERS];
unsigned char next[PLAYERS];
char angle[PLAYERS];
#define GEAR_ANIM_STEPS 2
char gear_anim[PLAYERS];

void draw_field( unsigned char x_pos, unsigned char y_pos, unsigned char player ) {
	unsigned char x,y,xp,yp,b=0;

	for( y=0 ; y < FIELD_TILES_V ; y++ ) {
		for( x=0 ; x < FIELD_TILES_H ; x++ ) {
			xp = x_pos+x;
			yp = y_pos+y;
			// Reasonable default...
			SetTile( xp, yp, BUBBLE_FIELD_TILE );
			if( y%2 == 0 ) {
				// Even row
				switch( x%3 ) {
					case 0:
						// Left-most tile
						if( bubbles[player][b] != C_BLANK ) {
							SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) );
						}
						break;
					case 1:
						// Split tile
						if( bubbles[player][b] == C_BLANK ) {
							SetTile( xp, yp, BUBBLE_FIRST_TILE + bubbles[player][b+1] + 1 );
						}
						else {
							if( b == 0 ) {
								SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + bubbles[player][b+1] + BUBBLE_EVEN_R_SPLIT );
							}
							else {
								SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1))
									+ bubbles[player][b+1] + BUBBLE_EVEN_R_SPLIT );
							}
						}
						b++;
						break;
					case 2:
						// Right-most tile
						if( bubbles[player][b] != C_BLANK ) {
							SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_EVEN_R_WHOLE );
						}
						b++;
						break;
				}
			}
			else {
				// Odd row
				switch( x%3 ) {
					case 0:
						if( (x == 0 || bubbles[player][b-1] == C_BLANK) && bubbles[player][b] != C_BLANK ) {
							SetTile( xp, yp, BUBBLE_SLIVER_L );
						}
						else if( bubbles[player][b] != C_BLANK ) {
							SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b-1]-1)) + BUBBLE_ODD_R );
						}
						break;
					case 1:
						if( bubbles[player][b] != C_BLANK ) {
							SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_ODD_MIDDLE );
						}
						b++;
						break;
					case 2:
						if( ( x == FIELD_TILES_H-1 || bubbles[player][b] == C_BLANK) && bubbles[player][b-1] != C_BLANK ) {
							SetTile( xp, yp, BUBBLE_SLIVER_R );
						}
						else {
							if( bubbles[player][b] != C_BLANK ) {
								SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_ODD_L );
							}
						}
						b++;
						break;							
				}
			}
		}
	}
}

int main(){
	SetTileTable(bg_tiles);
	SetSpritesTileTable(sprite_tiles);
	ClearVram();

//	bubbles[0][0] = C_RED;
	bubbles[0][1] = C_ORANGE;
	bubbles[0][2] = C_BLANK;
	bubbles[0][3] = C_GREEN;
	bubbles[0][4] = C_BLUE;
	bubbles[0][5] = C_BLANK;
	bubbles[0][6] = C_BLACK;
	bubbles[0][7] = C_RED;

	bubbles[0][8] = C_RED;
	bubbles[0][9] = C_BLANK;
	bubbles[0][10] = C_YELLOW;
//	bubbles[0][11] = C_GREEN;
	bubbles[0][12] = C_BLUE;
	bubbles[0][13] = C_PURPLE;
	bubbles[0][14] = C_BLACK;
	
	bubbles[0][15] = C_RED;

	draw_field( FIELD_OFFSET_X, FIELD_OFFSET_Y, 0 );
	DrawMap2( FIELD_OFFSET_X, FIELD_OFFSET_Y+FIELD_TILES_V, map_panel );
	
	draw_field( SCREEN_TILES_H - FIELD_TILES_H - FIELD_OFFSET_X -1, FIELD_OFFSET_Y, 1 );
	DrawMap2( SCREEN_TILES_H - FIELD_TILES_H - FIELD_OFFSET_X -1, FIELD_OFFSET_Y+FIELD_TILES_V, map_panel );

	sprites[0].tileIndex=15;
	sprites[1].tileIndex=16;
	sprites[2].tileIndex=17;

	while(1) {
		if( ReadJoypad(0) & BTN_LEFT ) {
			angle[0]--;
			if( angle[0] < -(ANGLES-1) ) {
				angle[0] = -(ANGLES-1);
			}
			gear_anim[0]--;
			if( gear_anim[0] < 0 ) {
				gear_anim[0] = GEAR_ANIM_STEPS-1;
			}
		}
		else if( ReadJoypad(0) & BTN_RIGHT ) {
			angle[0]++;
			if( angle[0] > ANGLES-1 ) {
				angle[0] = ANGLES-1;
			}
			gear_anim[0]++;
			if( gear_anim[0] > GEAR_ANIM_STEPS-1 ) {
				gear_anim[0] = 0;
			}
		}

		if( angle[0] >= 0 ) {
			sprites[0].x = ((FIELD_OFFSET_X + (FIELD_TILES_H/2))*TILE_WIDTH) + pgm_read_byte( arrow_x + angle[0] ) -4;
			sprites[1].x = ((FIELD_OFFSET_X + (FIELD_TILES_H/2))*TILE_WIDTH) + pgm_read_byte(  ring_x + angle[0] ) -4;
			sprites[2].x = ((FIELD_OFFSET_X + (FIELD_TILES_H/2))*TILE_WIDTH) + pgm_read_byte( rivet_x + angle[0] ) -4;

			sprites[0].y = ((FIELD_OFFSET_Y + FIELD_TILES_H)*TILE_HEIGHT) - pgm_read_byte( arrow_y + angle[0] ) -2;
			sprites[1].y = ((FIELD_OFFSET_Y + FIELD_TILES_H)*TILE_HEIGHT) - pgm_read_byte(  ring_y + angle[0] ) -5;
			sprites[2].y = ((FIELD_OFFSET_Y + FIELD_TILES_H)*TILE_HEIGHT) - pgm_read_byte( rivet_y + angle[0] ) -3;
		}
		else {
			sprites[0].x = ((FIELD_OFFSET_X + (FIELD_TILES_H/2))*TILE_WIDTH);
			sprites[0].x -= pgm_read_byte( arrow_x - angle[0] );
			sprites[0].x -= 4;
			sprites[1].x = ((FIELD_OFFSET_X + (FIELD_TILES_H/2))*TILE_WIDTH);
			sprites[1].x -= pgm_read_byte( ring_x - angle[0] );
			sprites[1].x -= 4;
			sprites[2].x = ((FIELD_OFFSET_X + (FIELD_TILES_H/2))*TILE_WIDTH);
			sprites[2].x -= pgm_read_byte( rivet_x - angle[0] );
			sprites[2].x -= 4;

			sprites[0].y = ((FIELD_OFFSET_Y + FIELD_TILES_H)*TILE_HEIGHT) - pgm_read_byte( arrow_y - angle[0] ) -2;
			sprites[1].y = ((FIELD_OFFSET_Y + FIELD_TILES_H)*TILE_HEIGHT) - pgm_read_byte(  ring_y - angle[0] ) -5;
			sprites[2].y = ((FIELD_OFFSET_Y + FIELD_TILES_H)*TILE_HEIGHT) - pgm_read_byte( rivet_y - angle[0] ) -3;
		}
		
		if( gear_anim[0] == 0 ) {
			DrawMap2( FIELD_OFFSET_X+2, FIELD_OFFSET_Y+FIELD_TILES_V, map_gears1 );
		}
		else {
			DrawMap2( FIELD_OFFSET_X+2, FIELD_OFFSET_Y+FIELD_TILES_V, map_gears2 );
		}

		WaitVsync(5);
	}
}

