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
	C_POP,
	C_COUNT
} colour_t;

#define FPS 60

// Number of first bubble tile in the set
// (not literally, as first row is for blanks).
#define BUBBLE_FIRST_TILE			0
#define BUBBLE_FIRST_COLOUR_TILE	12
// Background tile for play field
#define BUBBLE_FIELD_TILE			1
// Offsets for bubbles on even-numbered rows
#define BUBBLE_EVEN_L				0
#define BUBBLE_EVEN_R_SPLIT			1
#define BUBBLE_EVEN_R_WHOLE			10
// Offsets for bubbles on odd-numbered rows
#define BUBBLE_ODD_MIDDLE			11
#define BUBBLE_ODD_L				12
#define BUBBLE_ODD_R				13
#define BUBBLE_ODD_L_BLANK			14
#define BUBBLE_ODD_R_BLANK			15
// Number of bubble tiles of each colour
#define BUBBLES_PER_COLOUR			16
#define BUBBLE_SLIVER_L				10
#define BUBBLE_SLIVER_R				11

#define BG_SPACE_TILE		141
#define TILE_SHINE_TOP		42
#define TILE_SHINE_BOTTOM	43

#define FLIPPER_SPEED		2

#define BUBBLE_WIDTH		12
#define FIELD_OFFSET_X		2
#define FIELD_OFFSET_Y		2
// Offset of player 2's field from that of player 1.
#define P2_TILE_OFFSET		14
#define P2_PIXEL_OFFSET		(TILE_WIDTH*P2_TILE_OFFSET)

// Pixel offsets into player's fields
#define FIELD_OFFSET_1P		(((SCREEN_TILES_H-FIELD_TILES_H)/2)*TILE_WIDTH)
#define FIELD_CENTRE_1P		((SCREEN_TILES_H/2)*TILE_WIDTH)
#define FIELD_OFFSET_2P(p)	((FIELD_OFFSET_X*TILE_WIDTH) + (P2_PIXEL_OFFSET*(p)))
#define FIELD_CENTRE_2P(p)	(((FIELD_OFFSET_X + (FIELD_TILES_H/2))*TILE_WIDTH) + (P2_PIXEL_OFFSET*(p)))

#include "data/bg.inc"
#include "data/sprites.inc"
#include "data/title.inc"
#include "data/patches.inc"
#include "data/title_song.inc"
#define FIRST_TEXT_TILE		1

#define FIELD_BUBBLES_H		8
#define FIELD_BUBBLES_V		11
#define FIELD_TILES_H		12
#define FIELD_TILES_V		11
#define NUM_BUBBLES			(8*6)+(7*5)
#define BUBBLE_ROWS			FIELD_TILES_V
#define GEAR_ANIM_STEPS 	2

// Pre-calcutated co-ordinates of arrow parts
#define ANGLES 30
const char arrow_x[ANGLES] PROGMEM = {
	 0,  1,  3,  5,  7,  8,  9, 10, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28 };
const char arrow_y[ANGLES] PROGMEM = {
	31, 31, 31, 31, 31, 30, 30, 30, 29, 29, 28, 28, 26, 26,	24,
	23, 22, 22, 21, 20, 19, 18, 17, 15, 14, 13, 11, 10,  8,  6 };
	
const char ring_x[ANGLES] PROGMEM = {
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 13,
	14, 15, 16, 16, 17, 17, 18, 18, 19, 19, 19, 20, 20, 20, 20 };
const char ring_y[ANGLES] PROGMEM = {
	20, 20, 20, 20, 20, 19, 19, 19, 18, 18, 17, 17, 16, 16, 15,
	14, 13, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1 };

const char rivet_x[ANGLES] PROGMEM = {
	 0,  1,  1,  2,  3,  4,  4,  5,  6,  6,  7,  8,  8,  9,  9,
	10, 10, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 14, 14 };
const char rivet_y[ANGLES] PROGMEM = {
	14, 14, 14, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 10,
	10,  9,  9,  8,  8,  7,  6,  6,  5,  4,  4,  3,  2,  1,  1 };

// Same for projectile paths, but mutiplied by a trajectory factor.
#define TRAJ_SHIFT 4
const char traj_x[ANGLES] PROGMEM = {
	 0,  3,  5,  8, 10, 12, 15, 17, 20, 22, 24, 26, 28, 30, 32,
	34, 36, 37, 39, 40, 42, 43, 44, 45, 46, 46, 47, 47, 48, 48 };
const char traj_y[ANGLES] PROGMEM = {
	48, 48, 48, 47, 47, 46, 46, 45, 44, 43, 42, 40, 39, 37, 36,
	34, 32, 30, 28, 26, 24, 22, 20, 17, 15, 12, 10,  8,  5,  3 };


// Sprite constants/macros
#define TILE_ARROW		23
#define TILE_RING		15
#define TILE_RIVET		16

#define TILE_BUBBLE_L(c)	((c*2)-1)
#define TILE_BUBBLE_R(c)	(c*2)

#define SPRITE_ARROW		0
#define SPRITE_RING			2
#define SPRITE_RIVET		4
#define SPRITE_PROJ_L		6
#define SPRITE_PROJ_R		8

// Macros for common calculations
#define ROW_WIDTH(r)		((r)&1 ? 7 : 8)
#define FIRST_IN_ROW(r)		(((r/2)*15) + ((r&1)*8))

// Structures
typedef struct {
	char angle;
	int x;
	int y;
} projectile_t;

#define MASTER_VOLUME 127

// Globals
#define PLAYERS 2
unsigned char players = 1;
unsigned char bubbles[PLAYERS][NUM_BUBBLES];
unsigned char current[PLAYERS];
unsigned char next[PLAYERS];
char angle[PLAYERS];
projectile_t proj[PLAYERS];
bool firing[PLAYERS];
unsigned char block_left[PLAYERS];
unsigned char block_right[PLAYERS];
bool block_fire[PLAYERS];
#define POP_SPEED 10
unsigned char popping[PLAYERS];
long score[PLAYERS];
unsigned int frame = 0;
// For 1-player game only.
#define WOBBLE_SECONDS	5
#define WOBBLE_DELAY	(30*FPS*2)
int wobble_timer;
unsigned char drop;

typedef enum {
	ALIGN_LEFT=0,
	ALIGN_RIGHT
} align_t;

void text_write_number( char x, char y, unsigned long num, align_t align, unsigned char space_tile ) {
	char digits[15];
	int pos = 0;
	
	digits[pos] = 0;

	while(num > 0) {
		digits[pos] = num%10;
		num -= digits[pos];
		num /= 10;
		pos++;
	}

	if( align == ALIGN_RIGHT ) x-=(pos-1);

	if(pos == 0) {
		if( align == ALIGN_RIGHT ) x--;
		pos++;
	}
	while(--pos >= 0) {
		SetTile( x++, y, digits[pos] + space_tile );
	}
}

void text_write( char x, char y, const char *text ) {
	char *p = (char*)text;
	unsigned char t = 0;
	
	while( *p ) {
		if ( *p >= 'A' && *p <= 'Z' ) {
			t = *p - 'A' + 11;
		}
		else if( *p >= '0' && *p <= '9' ) {
			t = *p - '0' + 1;
		}
		else {
			switch( *p ) {
				case '.': t=37; break;
				case '!': t=38; break;
				case '/': t=39; break;
				case 'c': t=40; break;
				default: t = 0; // blank
			}
		}
		if( t ) {
			SetTile(x, y, t + FIRST_TEXT_TILE );
		}
		x++;
		p++;
	}
}

void draw_field( unsigned char player ) {
	unsigned char x,y,xp,yp,b=0;

	for( y=0 ; y+drop < FIELD_TILES_V ; y++ ) {
		for( x=0 ; x < FIELD_TILES_H ; x++ ) {
			if( players == 1 ) {
				xp = ((SCREEN_TILES_H-FIELD_TILES_H)/2) + x;
			}
			else {
				xp = FIELD_OFFSET_X + (P2_TILE_OFFSET*player) + x;
			}
			yp = FIELD_OFFSET_Y + y + drop;

			// Reasonable default...
			SetTile( xp, yp, BUBBLE_FIELD_TILE );
			if( (y&1) == 0 ) {
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
							SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1))
								+ bubbles[player][b+1] + BUBBLE_EVEN_R_SPLIT );
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
						if( (x == 0 || bubbles[player][b-1] == C_BLANK) && bubbles[player][b] != C_BLANK && bubbles[player][b] != C_POP ) {
							SetTile( xp, yp, BUBBLE_SLIVER_L );
						}
						else if( bubbles[player][b] != C_BLANK && bubbles[player][b] != C_POP ) {
							SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b-1]-1)) + BUBBLE_ODD_R );
						}
						else if( bubbles[player][b-1] != C_BLANK && bubbles[player][b-1] != C_POP && x != 0 ) {
							SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b-1]-1)) + BUBBLE_ODD_R_BLANK );
						}
						break;
					case 1:
						if( bubbles[player][b] != C_BLANK ) {
							SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_ODD_MIDDLE );
						}
						b++;
						break;
					case 2:
						if( ( x == FIELD_TILES_H-1 || bubbles[player][b] == C_BLANK) && bubbles[player][b-1] != C_BLANK && bubbles[player][b-1] != C_POP ) {
							SetTile( xp, yp, BUBBLE_SLIVER_R );
						}
						else if( x != FIELD_TILES_H-1 ) {
							if( bubbles[player][b] != C_BLANK ) {
								if( bubbles[player][b-1] == C_BLANK || bubbles[player][b-1] == C_POP ) {
									SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_ODD_L_BLANK );
								}
								else {
									SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_ODD_L );
								}
							}
						}
						if( x == FIELD_TILES_H-1 ) {
							b--;
						}
						b++;
						break;							
				}
			}
		}
	}
}

void new_bubble( unsigned char player ) {
	if( player < players ) {
		current[player] = next[player];
		next[player] = ((random()+frame)%(C_COUNT-2)) + 1;

		proj[player].x = ((FIELD_TILES_H*TILE_WIDTH)/2) - (BUBBLE_WIDTH/2);
		proj[player].y = ((FIELD_TILES_V+1)*TILE_HEIGHT) - (BUBBLE_WIDTH/2);
	
		proj[player].x <<= TRAJ_SHIFT;
		proj[player].y <<= TRAJ_SHIFT;

		sprites[SPRITE_PROJ_L+player].tileIndex = TILE_BUBBLE_L( current[(int)player] );
		sprites[SPRITE_PROJ_R+player].tileIndex = TILE_BUBBLE_R( current[(int)player] );

		if( players == 1 ) {
			SetTile( ((SCREEN_TILES_H-FIELD_TILES_H)/2) + FIELD_TILES_H - 2, FIELD_OFFSET_Y + FIELD_TILES_V + 1,
				BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(next[player]-1)) + BUBBLE_ODD_L_BLANK);
			SetTile( ((SCREEN_TILES_H-FIELD_TILES_H)/2) + FIELD_TILES_H - 1, FIELD_OFFSET_Y + FIELD_TILES_V + 1,
				BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(next[player]-1)) + BUBBLE_ODD_R_BLANK);
		}
		else {
			SetTile( FIELD_OFFSET_X + (P2_TILE_OFFSET*player) + FIELD_TILES_H - 2, FIELD_OFFSET_Y + FIELD_TILES_V + 1,
				BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(next[player]-1)) + BUBBLE_ODD_L_BLANK);
			SetTile( FIELD_OFFSET_X + (P2_TILE_OFFSET*player) + FIELD_TILES_H - 1, FIELD_OFFSET_Y + FIELD_TILES_V + 1,
				BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(next[player]-1)) + BUBBLE_ODD_R_BLANK);			
		}
	}
}

bool drop_bubbles( unsigned char player ) {
	int i;
	bool bottomed_out = false;
	unsigned char last_row = BUBBLE_ROWS-1-drop;
	
	// Check if lowest row had bubbles.
	for( i = FIRST_IN_ROW(last_row) ; i < FIRST_IN_ROW(last_row)+ROW_WIDTH(last_row) ; i++ ) {
		if( bubbles[player][i] != C_BLANK ) {
			bottomed_out = true;
		}
		bubbles[player][i] = C_BLANK;
	}

	drop++;
	if( drop >= BUBBLE_ROWS ) {
		// Fell of bottom of screen...
		bottomed_out = 1;
	}
	
	return bottomed_out;
}

bool do_wobble( void ) {
	int second = wobble_timer/FPS;
	bool bottomed_out = false;
	
	if( second < WOBBLE_SECONDS ) {
		int step = FPS/(second+2);
		if( wobble_timer % step == 0 ) {
			DrawMap2( (SCREEN_TILES_H-FIELD_TILES_H)/2, FIELD_OFFSET_Y+drop-1, map_drop_bar_shake );
		}
		else if( wobble_timer % step == step/2 ) {
			DrawMap2( (SCREEN_TILES_H-FIELD_TILES_H)/2, FIELD_OFFSET_Y+drop-1, map_drop_bar_normal );
		}
	}
	else {
		bottomed_out = drop_bubbles(0);
		
		draw_field(0);
		DrawMap2( (SCREEN_TILES_H-FIELD_TILES_H)/2, FIELD_OFFSET_Y+drop-2, map_drop_bar_clear );
		DrawMap2( (SCREEN_TILES_H-FIELD_TILES_H)/2, FIELD_OFFSET_Y+drop-1, map_drop_bar_normal );
		
		wobble_timer = -WOBBLE_DELAY;
	}
	return bottomed_out;
}

bool proc_controls( unsigned char player ) {
	bool changed = false;
	int buttons = ReadJoypad(player);
	
	if( block_left[player]  ) block_left[player]--;
	if( block_right[player] ) block_right[player]--;
	
	if( buttons & BTN_LEFT ) {
		if( !block_left[player] ) {
			// Rotate left
			TriggerFx( PATCH_TICK, 0x80, true );
			angle[(int)player]--;
			if( angle[(int)player] < -(ANGLES-1) ) {
				angle[(int)player] = -(ANGLES-1);
			}
			changed = true;
			block_left[player] = 5;
			block_right[player] = 0;
		}
	}
	else {
		block_left[player] = 0;
	}
	
	if( buttons & BTN_RIGHT ) {
		if( !block_right[player] ) {
			// Rotate right
			TriggerFx( PATCH_TICK, 0x80, true );
			angle[(int)player]++;
			if( angle[(int)player] > ANGLES-1 ) {
				angle[(int)player] = ANGLES-1;
			}
			changed = true;
			block_right[player] = 5;
			block_left[player] = 0;
		}
	}
	else {
		block_right[player] = 0;
	}
	
	if( buttons & (BTN_A|BTN_B|BTN_X|BTN_Y) ) {
		if( !block_fire[player] ) {
			if( !firing[player] ) {
				// Fire!
				proj[player].angle = angle[player];
				firing[player] = true;
				TriggerFx( PATCH_SHOOT, 0xff, true );
			}
			block_fire[player] = true;
		}
	}
	else {
		block_fire[player] = false;
	}

	return changed;
}

void draw_arrow( unsigned char x, unsigned char y, unsigned char player ) {
	if( angle[player] >= 0 ) {
		sprites[SPRITE_ARROW+player].tileIndex = TILE_ARROW + ((angle[player]+2)/5);
	
		sprites[SPRITE_ARROW+player].x = x + pgm_read_byte( arrow_x + angle[player] ) -4;
		sprites[SPRITE_RING+player ].x = x + pgm_read_byte(  ring_x + angle[player] ) -4;
		sprites[SPRITE_RIVET+player].x = x + pgm_read_byte( rivet_x + angle[player] ) -4;

		sprites[SPRITE_ARROW+player].y = y - pgm_read_byte( arrow_y + angle[player] ) -2;
		sprites[SPRITE_RING+player ].y = y - pgm_read_byte(  ring_y + angle[player] ) -5;
		sprites[SPRITE_RIVET+player].y = y - pgm_read_byte( rivet_y + angle[player] ) -3;
	}
	else {
		sprites[SPRITE_ARROW+player].tileIndex = TILE_ARROW + ((angle[player]-2)/5);

		sprites[SPRITE_ARROW+player].x = x - pgm_read_byte( arrow_x - angle[player] ) -4;
		sprites[SPRITE_RING+player ].x = x - pgm_read_byte(  ring_x - angle[player] ) -4;
		sprites[SPRITE_RIVET+player].x = x - pgm_read_byte( rivet_x - angle[player] ) -4;

		sprites[SPRITE_ARROW+player].y = y - pgm_read_byte( arrow_y - angle[player] ) -2;
		sprites[SPRITE_RING+player ].y = y - pgm_read_byte(  ring_y - angle[player] ) -5;
		sprites[SPRITE_RIVET+player].y = y - pgm_read_byte( rivet_y - angle[player] ) -3;
	}
}

void set_score( unsigned char player, long s ) {
#define MAX_SCORE 99999999
	score[player] = s;
	if( score[player] > MAX_SCORE ) score[player] = MAX_SCORE;

	if( players == 1 ) {
		text_write_number( 18, 16, score[player], ALIGN_RIGHT, BG_SPACE_TILE );
	}
	else {
		text_write_number( 11+(P2_TILE_OFFSET*player), 16, score[player], ALIGN_RIGHT, BG_SPACE_TILE );
	}
}

void draw_projectile( unsigned char player ) {
	if( players == 1 ) {
		sprites[SPRITE_PROJ_L+player].x = FIELD_OFFSET_1P + (proj[player].x>>TRAJ_SHIFT);
	}
	else {
		sprites[SPRITE_PROJ_L+player].x = FIELD_OFFSET_2P(player) + (proj[player].x>>TRAJ_SHIFT);
	}
	sprites[SPRITE_PROJ_R+player].x = sprites[SPRITE_PROJ_L+player].x + TILE_WIDTH;

	sprites[SPRITE_PROJ_L+player].y = (FIELD_OFFSET_Y*TILE_HEIGHT) + (proj[player].y>>TRAJ_SHIFT);
	sprites[SPRITE_PROJ_R+player].y = sprites[SPRITE_PROJ_L+player].y;
}


bool check_links( unsigned char player, int b ) {
	// Check for links of three or more bubbles from bubble "b".
	popping[player] = 1;
	return (popping[player] != 0);
}

#define PROJ_ROW(y) (((y)/BUBBLE_WIDTH)-drop)

unsigned char proj_column( int x, unsigned char row ) {
	if( row&1 ) {
		// Odd row, 7 bubbles.
		char column = (x-(BUBBLE_WIDTH/2)) / BUBBLE_ROWS;
		if( column > 6 ) return 6;
		return column;
	}
	return x/BUBBLE_ROWS;
}

bool update_projectile( unsigned char player ) {
	bool bottomed_out = false;
	unsigned hit = 0;
	int top, bottom, left, right;
	unsigned char row;
	int candidate;

	if( firing[player] ) {
		proj[player].y -= pgm_read_byte( traj_y + proj[player].angle );
	
		if( proj[player].angle >= 0 ) {
			int edge = ((FIELD_TILES_H*TILE_WIDTH)-BUBBLE_WIDTH) << TRAJ_SHIFT;
			proj[player].x += pgm_read_byte( traj_x + proj[player].angle );
			if( proj[player].x >= edge ) {
				proj[player].x = edge - (proj[player].x-edge);
				proj[player].angle = -proj[player].angle;
			}
		}
		else {
			proj[player].x -= pgm_read_byte( traj_x - proj[player].angle );
			if( proj[player].x < 0 ) {
				proj[player].x = 0 - proj[player].x;
				proj[player].angle = -proj[player].angle;
			}
		}
	}
	
	// Collision check
#define BORDER 3
#define CENTRE(x) ((x)-BORDER+(BUBBLE_WIDTH/2))
	top = (proj[player].y>>TRAJ_SHIFT) + BORDER;
	bottom = top + BUBBLE_WIDTH - (BORDER*2);
	left = (proj[player].x>>TRAJ_SHIFT) + BORDER;
	right = left + BUBBLE_WIDTH - (BORDER*2);

#define HIT_TOP		0x01
#define HIT_BOTTOM	0x02
#define HIT_LEFT	0x04
#define HIT_RIGHT	0x08
#define HIT_LIMIT	0x10
	if( top - BORDER <= (drop * BUBBLE_WIDTH) ) {
		hit |= HIT_LIMIT;
	}

	row = PROJ_ROW( top );
	candidate = FIRST_IN_ROW( row ) + proj_column( left, row );
	if( candidate < NUM_BUBBLES && bubbles[player][candidate] != C_BLANK ) hit |= HIT_TOP;

	row = PROJ_ROW( top );
	candidate = FIRST_IN_ROW( row ) + proj_column( right, row );
	if( candidate < NUM_BUBBLES && bubbles[player][candidate] != C_BLANK ) hit |= HIT_BOTTOM;

	row = PROJ_ROW( bottom );
	candidate = FIRST_IN_ROW( row ) + proj_column( left, row );
	if( candidate < NUM_BUBBLES && bubbles[player][candidate] != C_BLANK ) hit |= HIT_LEFT;

	row = PROJ_ROW( bottom );
	candidate = FIRST_IN_ROW( row ) + proj_column( right, row );
	if( candidate < NUM_BUBBLES && bubbles[player][candidate] != C_BLANK ) hit |= HIT_RIGHT;

	if( hit ) {
		row = PROJ_ROW( CENTRE(top) );
		candidate = FIRST_IN_ROW( row ) + proj_column( CENTRE(left), row );

		if( candidate < NUM_BUBBLES && bubbles[player][candidate] != C_BLANK ) {
			if( hit & HIT_TOP ) {
				row = PROJ_ROW( bottom );
			}
			else {
				row = PROJ_ROW( top );
			}

			candidate = FIRST_IN_ROW( row );

			if( hit & HIT_LEFT ) {
				candidate += proj_column( right, row );
			}
			else {
				candidate += proj_column( left, row );
			}
		}

		if( candidate >= NUM_BUBBLES || row + drop >= BUBBLE_ROWS ) {
			bottomed_out = true;
		}
		else {
			bubbles[player][candidate] = current[player];
			draw_field( player );
			if( check_links( player, candidate ) ) {
				popping[player] = POP_SPEED;
			}

			new_bubble( player );
			firing[player] = false;
		}	
	}

	if( bottomed_out ) {
		// Place the projectile where it would have been if it could be drawn as a tile.
		proj[player].y = (BUBBLE_ROWS * BUBBLE_WIDTH) << TRAJ_SHIFT;
		proj[player].x = (proj_column( (proj[player].x>>TRAJ_SHIFT)+(BUBBLE_WIDTH/2), row ) * BUBBLE_WIDTH) << TRAJ_SHIFT;
		if( row & 1 ) {
			proj[player].x += (BUBBLE_WIDTH/2) << TRAJ_SHIFT;
		}
	}

	draw_projectile( player );
	return bottomed_out;
}

void update_arrow( unsigned char player ) {
	if( player < players ) {
		if( players == 1 ) {
			draw_arrow(	FIELD_CENTRE_1P, ((FIELD_OFFSET_Y + FIELD_TILES_H)*TILE_HEIGHT), player );

			if( angle[player] % GEAR_ANIM_STEPS == 0 ) {
				DrawMap2( ((SCREEN_TILES_H-FIELD_TILES_H)/2) , FIELD_OFFSET_Y+FIELD_TILES_V, map_gears1 );
			}
			else {
				DrawMap2( ((SCREEN_TILES_H-FIELD_TILES_H)/2), FIELD_OFFSET_Y+FIELD_TILES_V, map_gears2 );
			}
		}
		else {
			draw_arrow( FIELD_CENTRE_2P(player), (FIELD_OFFSET_Y + FIELD_TILES_H) * TILE_HEIGHT, player );

			if( angle[player] % GEAR_ANIM_STEPS == 0 ) {
				DrawMap2( FIELD_OFFSET_X + (P2_TILE_OFFSET*player), FIELD_OFFSET_Y+FIELD_TILES_V, map_gears1 );
			}
			else {
				DrawMap2( FIELD_OFFSET_X + (P2_TILE_OFFSET*player), FIELD_OFFSET_Y+FIELD_TILES_V, map_gears2 );
			}
		}
	}
}

void draw_map_flipper( unsigned char xp, unsigned char yp, const char *map ) {
	int w = pgm_read_byte( map );
	int h = pgm_read_byte( map+1 );
	int x,y;
	
	for( x=1 ; x<w ; x+=2 ) {
		for( y=0 ; y<h ; y++ ) {
			SetTile( xp+x, yp+y, pgm_read_byte( map + 2 + (y*w)+x ) );
		}
		WaitVsync(FLIPPER_SPEED);
		for( y=0 ; y<h ; y++ ) {
			SetTile( xp+x-1, yp+y, pgm_read_byte( map + 2 + (y*w)+x - 1 ) );
		}
		WaitVsync(FLIPPER_SPEED);
	}
}

void clear_screen_flipper( bool fade_audio ) {
	int x,y;

	for( x=1 ; x<SCREEN_TILES_H ; x+=2 ) {
		WaitVsync(FLIPPER_SPEED);
		for( y=0 ; y<SCREEN_TILES_V ; y++ ) {
			SetTile( x, y, 0 );
		}
		if( fade_audio ) SetMasterVolume( MASTER_VOLUME - (MASTER_VOLUME/FLIPPER_SPEED) );

		WaitVsync(FLIPPER_SPEED);
		for( y=0 ; y<SCREEN_TILES_V ; y++ ) {
			SetTile( x-1, y, 0 );
		}
		if( fade_audio ) SetMasterVolume( MASTER_VOLUME - (MASTER_VOLUME/FLIPPER_SPEED) );
	}

	ClearVram();
	if( fade_audio ) SetMasterVolume( 0 );
}

void draw_bg( unsigned char bg_frame ) {
	int x,y;

	for( x=0 ; x<5 ; x++ ) {
		for( y=0 ; y<3 ; y++ ) {
			DrawMap2( x*6, y*6, map_bg0 + (sizeof(map_bg0)*(bg_frame)) );
		}
	}
}

int main(){
	int p = 0;
	bool game_over;
	unsigned char loser = 0;

	InitMusicPlayer(patches);

	while(1) {
		SetTileTable(title_tiles);
		SetSpritesTileTable(sprite_tiles);
		ClearVram();
		SetSpriteVisibility(false);
		for( p=0 ; p < MAX_SPRITES ; p++ ) {
			sprites[p].x = SCREEN_TILES_H*TILE_WIDTH;
		}

		frame = 0;
		p = 1;
		SetMasterVolume( MASTER_VOLUME );
		StartSong( title_song );
		WaitVsync(60);
		FadeIn(1,false);
		while( 1 ) {
			unsigned char shine_offset;
			WaitVsync(3);
			draw_bg( frame % 12 );
			DrawMap2( 3,4, map_title );

			shine_offset = frame%(FPS+(FPS/2));
			if( shine_offset < 22 ) {
				SetTile( 4+shine_offset, 4, TILE_SHINE_TOP );
			}
			if( shine_offset > 1 && shine_offset < 24 ) {
				SetTile( 4+shine_offset-2, 4, TILE_SHINE_TOP );
			}
			if( shine_offset >= 7 && shine_offset < 29 ) {
				SetTile( 4+shine_offset-7, 8, TILE_SHINE_BOTTOM );
			}
			if( shine_offset >= 9 && shine_offset < 31 ) {
				SetTile( 4+shine_offset-7-2, 8, TILE_SHINE_BOTTOM );
			}

			if( frame % FPS < FPS/2 ) {
				text_write( 10,12, "PUSH START" );
			}
			text_write( 5,16, "c2011 STEVE MADDISON" );

			frame++;
			if( frame % (FPS*12) == 0 ) frame = 0;

			if( ReadJoypad(0) & BTN_START ) {
				if( !p ) break;
			}
			else {
				p = 0;
			}
		}

		// Select number of players...
		p = 1;
		while(1) {
			unsigned char shine_offset;
			int buttons;
			WaitVsync(3);
			draw_bg( frame % 12 );
			frame++;
			if( frame % (FPS*12) == 0 ) frame = 0;

			if( players == 1 ) {
				DrawMap2( 2, 4, map_player_selected );
				DrawMap2( 2+P2_TILE_OFFSET, 4, map_player_deselected );
			}
			else {
				DrawMap2( 2, 4, map_player_deselected );
				DrawMap2( 2+P2_TILE_OFFSET, 4, map_player_selected );
			}
			DrawMap2( 4, 5, map_1_player );
			DrawMap2( 4+P2_TILE_OFFSET, 5, map_2_player );

			text_write( 8,14, "SELECT PLAYERS" );

			shine_offset = frame%(FPS+(FPS/2));
			if( shine_offset < 10 ) {
				SetTile( 3+shine_offset+(P2_TILE_OFFSET*(players-1)), 4, TILE_SHINE_TOP );
			}
			if( shine_offset > 1 && shine_offset < 12 ) {
				SetTile( 3+shine_offset+(P2_TILE_OFFSET*(players-1))-2, 4, TILE_SHINE_TOP );
			}
			if( shine_offset >= 7 && shine_offset < 17 ) {
				SetTile( 3+shine_offset+(P2_TILE_OFFSET*(players-1))-7, 10, TILE_SHINE_BOTTOM );
			}
			if( shine_offset >= 9 && shine_offset < 19 ) {
				SetTile( 3+shine_offset+(P2_TILE_OFFSET*(players-1))-7-2, 10, TILE_SHINE_BOTTOM );
			}

			buttons = ReadJoypad(0);
			if( buttons & BTN_LEFT ) {
				players = 1;
			}
			else if ( buttons & BTN_RIGHT ) {
				players = 2;
			}
			else if ( buttons & BTN_SELECT ) {
				// "p" used as input "lock" here.
				if( !p ) {
					players++;
					if( players > 2 ) players = 1;
					p = 1;
				}
			}
			else if( buttons & (BTN_START|BTN_A|BTN_B|BTN_X|BTN_Y) ) {
				if( !p ) break;
			}
			else {
				p = 0;
			}
		};

		clear_screen_flipper( true );
		StopSong();
		SetTileTable(bg_tiles);

		for( p = 0 ; p < NUM_BUBBLES ; p++ ) {
			bubbles[0][p] = C_BLANK;
			bubbles[1][p] = C_BLANK;
		}
		for( p = 0 ; p < (3*8)+(2*7) ; p++ ) {
			bubbles[0][p] = random()%(C_COUNT-1);
			bubbles[1][p] = random()%(C_COUNT-1);
		}

		drop = 0;

		if( players == 1 ) {
			draw_map_flipper( 0, 0, map_field_1p );
			draw_field(0);
		}
		else {
			draw_map_flipper( 0, 0, map_field_2p );
			draw_field(0);
			draw_field(1);
		}

		for( p=0 ; p<PLAYERS ; p++ ) {
			if( p < players ) {
				// Tile indices of arrow parts.
				sprites[SPRITE_ARROW+p].tileIndex = TILE_ARROW;
				sprites[SPRITE_RING +p].tileIndex = TILE_RING;
				sprites[SPRITE_RIVET+p].tileIndex = TILE_RIVET;

				firing[p] = false;	
				popping[p] = 0;
				new_bubble(p); // Initialize next	
				new_bubble(p); // Initialise current and next
				draw_projectile(p);
		
				angle[p] = 0;
				update_arrow(p);
				set_score( p, 0 );
			}
		}

		wobble_timer = -WOBBLE_DELAY;
		game_over = false;
	
		SetSpriteVisibility(true);
		SetMasterVolume( MASTER_VOLUME );
		StartSong( title_song );

		while(!game_over) {
			if( frame & 1 ) WaitVsync(1);

			for( p=0 ; p < players ; p++ ) {
				if( proc_controls(p) ) {
					update_arrow(p);
				}

				if( popping[p] ) {
					popping[p]--;
					if( popping[p] == 0 ) {
						int i;
						for( i=0 ; i<NUM_BUBBLES ; i++ ) {
							if( bubbles[p][i] == C_POP ) {
								bubbles[p][i] = C_BLANK;
							}
						}
						draw_field( p );
					}
				}
				else if( firing[p] ) {
					game_over = update_projectile(p);
					if( game_over ) loser = p;
				}

				if( players == 1 && ++wobble_timer > 0 ) {
					game_over = do_wobble();
				}
			}

			frame++;
		}

		// Game over
		StopSong();
		if( players == 1 ) {
			DrawMap2( (SCREEN_TILES_H-FIELD_TILES_H)/2, FIELD_OFFSET_Y+(FIELD_TILES_V/2)-2, map_lose );
			TriggerFx( PATCH_LOSE, 0xff, true );
		}
		else {
			if( loser == 0 ) {
				DrawMap2( FIELD_OFFSET_X, FIELD_OFFSET_Y+(FIELD_TILES_V/2)-2, map_lose );
				DrawMap2( FIELD_OFFSET_X+P2_TILE_OFFSET, FIELD_OFFSET_Y+(FIELD_TILES_V/2)-2, map_win );
				if( firing[1] ) {
					// Hide opponent's projectile.
					sprites[SPRITE_PROJ_L+1].tileIndex = 0;
					sprites[SPRITE_PROJ_R+1].tileIndex = 0;
				}
			}
			else {
				DrawMap2( FIELD_OFFSET_X, FIELD_OFFSET_Y+(FIELD_TILES_V/2)-2, map_win );
				DrawMap2( FIELD_OFFSET_X+P2_TILE_OFFSET, FIELD_OFFSET_Y+(FIELD_TILES_V/2)-2, map_lose );
				if( firing[0] ) {
					sprites[SPRITE_PROJ_L].tileIndex = 0;
					sprites[SPRITE_PROJ_R].tileIndex = 0;
				}
			}
			TriggerFx( PATCH_WIN1, 0xff, true );
			TriggerFx( PATCH_WIN2, 0xff, true );
		}
		WaitVsync(60);
		
		while( ReadJoypad(0) != 0 || ReadJoypad(1) != 0 );
		while( ReadJoypad(0) == 0 && ReadJoypad(1) == 0 );
		SetSpriteVisibility(false);
		clear_screen_flipper( true );
	}
}

