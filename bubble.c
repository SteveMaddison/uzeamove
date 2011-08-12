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

#include "data/bg.inc"
#include "data/sprites.inc"

#define PLAYERS				2
#define FIELD_BUBBLES_H		8
#define FIELD_BUBBLES_V		11
#define FIELD_TILES_H		12
#define FIELD_TILES_V		11
#define NUM_BUBBLES			83

// Globals
unsigned char bubbles[PLAYERS][NUM_BUBBLES];
unsigned char current[PLAYERS];
unsigned char next[PLAYERS];


void draw_field( unsigned char x_pos, unsigned char y_pos, unsigned char player ) {
	unsigned char x,y,b=0;

	for( y=0 ; y < FIELD_TILES_V ; y++ ) {
		for( x=0 ; x < FIELD_TILES_H ; x++ ) {
			// Reasonable default...
			SetTile( x, y, BUBBLE_FIELD_TILE );
			if( y%2 == 0 ) {
				// Even row
				switch( x%3 ) {
					case 0:
						// Left-most tile
						if( bubbles[player][b] != C_BLANK ) {
							SetTile( x, y, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) );
						}
						break;
					case 1:
						// Split tile
						if( bubbles[player][b] != C_BLANK ) {
							if( b == 0 ) {
								SetTile( x, y, BUBBLE_FIRST_COLOUR_TILE + bubbles[player][b+1] + BUBBLE_EVEN_R_SPLIT );
							}
							else {
								SetTile( x, y, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + bubbles[player][b+1] + BUBBLE_EVEN_R_SPLIT );
							}
						}
						b++;
						break;
					case 2:
						// Right-most tile
						if( bubbles[player][b] != C_BLANK ) {
							SetTile( x, y, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_EVEN_R_WHOLE );
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
							SetTile( x, y, BUBBLE_SLIVER_L );
						}
						else if( bubbles[player][b] != C_BLANK ) {
							SetTile( x, y, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b-1]-1)) + BUBBLE_ODD_R );
						}
						break;
					case 1:
						if( bubbles[player][b] != C_BLANK ) {
							SetTile( x, y, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_ODD_MIDDLE );
						}
						b++;
						break;
					case 2:
						if( ( x == FIELD_TILES_H-1 || bubbles[player][b] == C_BLANK) && bubbles[player][b-1] != C_BLANK ) {
							SetTile( x, y, BUBBLE_SLIVER_R );
						}
						else {
							if( bubbles[player][b] != C_BLANK ) {
								SetTile( x, y, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_ODD_L );
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
	ClearVram();

	bubbles[0][0] = C_RED;
	bubbles[0][1] = C_ORANGE;
	bubbles[0][2] = C_YELLOW;
	bubbles[0][3] = C_GREEN;
	bubbles[0][4] = C_BLUE;
	bubbles[0][5] = C_PURPLE;
	bubbles[0][6] = C_BLACK;
	bubbles[0][7] = C_RED;

	bubbles[0][8] = C_RED;
	bubbles[0][9] = C_ORANGE;
	bubbles[0][10] = C_YELLOW;
//	bubbles[0][11] = C_GREEN;
	bubbles[0][12] = C_BLUE;
	bubbles[0][13] = C_PURPLE;
	bubbles[0][14] = C_BLACK;
	
	bubbles[0][15] = C_RED;

	draw_field( 0,0,0 );

	while(1) {
	}
}

