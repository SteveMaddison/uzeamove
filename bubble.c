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

#define FPS 60

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
#define BUBBLE_ODD_L_BLANK			13
#define BUBBLE_ODD_R_BLANK			14
// Number of bubble tiles of each colour
#define BUBBLES_PER_COLOUR			15
#define BUBBLE_SLIVER_L				9
#define BUBBLE_SLIVER_R				10

#define BG_SPACE_TILE		117

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
#ifndef TITLE_MISSING
#include "data/title.inc"
#endif
#include "data/title2.inc"
#define FIRST_TEXT_TILE		1

#define FIELD_BUBBLES_H		8
#define FIELD_BUBBLES_V		11
#define FIELD_TILES_H		12
#define FIELD_TILES_V		11
#define NUM_BUBBLES			83
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
#define TRAJ_FACTOR 8
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
#define ROW_WIDTH(r)		((r)%2 ? 7 : 8)
#define FIRST_IN_ROW(r)		(((r/2)*15) + ((r%2)*8))

// Structures
typedef struct {
	char angle;
	int x;
	int y;
} projectile_t;


// Globals
#define PLAYERS 2
unsigned char players = 2;
unsigned char bubbles[PLAYERS][NUM_BUBBLES];
unsigned char current[PLAYERS];
unsigned char next[PLAYERS];
char angle[PLAYERS];
projectile_t proj[PLAYERS];
bool firing[PLAYERS];
unsigned char block_left[PLAYERS];
unsigned char block_right[PLAYERS];
long score[PLAYERS];
unsigned int frame = 0;
// For 1-player game only.
#define WOBBLE_SECONDS	5
#define WOBBLE_DELAY	(30*FPS)
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
						if( (x == 0 || bubbles[player][b-1] == C_BLANK) && bubbles[player][b] != C_BLANK ) {
							SetTile( xp, yp, BUBBLE_SLIVER_L );
						}
						else if( bubbles[player][b] != C_BLANK ) {
							SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b-1]-1)) + BUBBLE_ODD_R );
						}
						else if( bubbles[player][b-1] != C_BLANK && x != 0 ) {
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
						if( ( x == FIELD_TILES_H-1 || bubbles[player][b] == C_BLANK) && bubbles[player][b-1] != C_BLANK ) {
							SetTile( xp, yp, BUBBLE_SLIVER_R );
						}
						else if( x != FIELD_TILES_H-1 ) {
							if( bubbles[player][b] != C_BLANK ) {
								if( bubbles[player][b-1] == C_BLANK ) {
									SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_ODD_L_BLANK );
								}
								else {
									SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_ODD_L );
								}
							}
						}
						else {
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
		next[player] = ((random()+frame)%(C_COUNT-1)) + 1;

		proj[player].x = ((FIELD_TILES_H*TILE_WIDTH)/2) - (BUBBLE_WIDTH/2);
		proj[player].y = ((FIELD_TILES_V+1)*TILE_HEIGHT) - (BUBBLE_WIDTH/2);
	
		proj[player].x *= TRAJ_FACTOR;
		proj[player].y *= TRAJ_FACTOR;

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
			angle[(int)player]--;
			if( angle[(int)player] < -(ANGLES-1) ) {
				angle[(int)player] = -(ANGLES-1);
			}
			changed = true;
			block_left[player] = 3;
			block_right[player] = 0;
		}
	}
	else {
		block_left[player] = 0;
	}
	
	if( buttons & BTN_RIGHT ) {
		if( !block_right[player] ) {
			// Rotate right
			angle[(int)player]++;
			if( angle[(int)player] > ANGLES-1 ) {
				angle[(int)player] = ANGLES-1;
			}
			changed = true;
			block_right[player] = 3;
			block_left[player] = 0;
		}
	}
	else {
		block_right[player] = 0;
	}
	
	if( buttons & (BTN_A|BTN_B|BTN_X|BTN_Y) ) {
		if( !firing[player] ) {
			// Fire!
			proj[player].angle = angle[player];
			firing[player] = true;
		}
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
		sprites[SPRITE_PROJ_L+player].x = FIELD_OFFSET_1P + (proj[player].x/TRAJ_FACTOR);
	}
	else {
		sprites[SPRITE_PROJ_L+player].x = FIELD_OFFSET_2P(player) + (proj[player].x/TRAJ_FACTOR);
	}
	sprites[SPRITE_PROJ_R+player].x = sprites[SPRITE_PROJ_L+player].x + TILE_WIDTH;

	sprites[SPRITE_PROJ_L+player].y = (FIELD_OFFSET_Y*TILE_HEIGHT) + (proj[player].y/TRAJ_FACTOR);
	sprites[SPRITE_PROJ_R+player].y = sprites[SPRITE_PROJ_L+player].y;
}

void update_projectile( unsigned char player ) {
	unsigned char row;

	if( firing[player] ) {
		proj[player].y -= pgm_read_byte( traj_y + proj[player].angle );
	
		if( proj[player].angle >= 0 ) {
			int edge = ((FIELD_TILES_H*TILE_WIDTH)-BUBBLE_WIDTH) * TRAJ_FACTOR;
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
	row = (proj[player].y/TRAJ_FACTOR/BUBBLE_ROWS) - drop;
	if( row < BUBBLE_ROWS ) {
		unsigned char column;

		if( angle[player] >= 0 ) {
			// Moving right, check top right corner.
			column = ( (proj[player].x/TRAJ_FACTOR) + (BUBBLE_WIDTH-1) -(row%2 * (BUBBLE_WIDTH/2)) )/BUBBLE_WIDTH;
		}
		else {
			// Moving left, check top left corner.
			column = ( (proj[player].x/TRAJ_FACTOR) - (row%2 * (BUBBLE_WIDTH/2)) )/BUBBLE_WIDTH;
		}

		if( row%2 && column > 6 ) {
			// Odd rows have less columns.
			column = 6;
		}

		if( bubbles[player][ FIRST_IN_ROW(row) + column ] != C_BLANK ) {
			row++;
			bubbles[player][ FIRST_IN_ROW(row) + column ] = current[player];

			draw_field( player );

			new_bubble( player );
			firing[player] = false;
		}
	}

	draw_projectile( player );
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

void clear_screen_flipper( void ) {
	int x,y;
	
	for( x=1 ; x<SCREEN_TILES_H ; x+=2 ) {
		for( y=0 ; y<SCREEN_TILES_V ; y++ ) {
			SetTile( x, y, 0 );
		}
		WaitVsync(FLIPPER_SPEED);
		for( y=0 ; y<SCREEN_TILES_V ; y++ ) {
			SetTile( x-1, y, 0 );
		}
		WaitVsync(FLIPPER_SPEED);
	}

	ClearVram();
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

	while(ReadJoypad(0));
	while(1) {	
#ifndef TITLE_MISSING
		SetTileTable(title_tiles);
#endif
		SetTileTable(title2_tiles);
		SetSpritesTileTable(sprite_tiles);
		ClearVram();
		SetSpriteVisibility(false);

		frame = 0;
		while( !(ReadJoypad(0) & BTN_START) ) {
			WaitVsync(2);
			draw_bg( frame % 12 );
			DrawMap2( 4,4, map_title );
			if( frame % FPS < FPS/2 ) {
				text_write( 10,12, "PUSH START" );
			}
			text_write( 5,16, "c2011 STEVE MADDISON" );
			frame++;
			if( frame % (FPS*12) == 0 ) frame = 0;
		}

#ifndef TITLE_MISSING
		// Select number of players...
		draw_map_flipper( 0, 0, map_player_select );
		do {
			int buttons;
		
			DrawMap2( 0, 0, map_player_select );
			if( players == 1 ) {
				DrawMap2( 2, 4, map_player_selector );
				DrawMap2( 4, 6, map_1_player );
			}
			else {
				DrawMap2( 16, 4, map_player_selector );
				DrawMap2( 18, 6, map_2_player );
			}
		
			while(ReadJoypad(0));
		
			buttons = 0;
			while( !buttons ) {
				 buttons = ReadJoypad(0);
			}
			if( buttons & (BTN_LEFT|BTN_RIGHT) ) {
				players++;
				if( players > 2 ) players = 1;
			}
			else if( buttons & BTN_START ) {
				p = 1;
			}
		} while( p==0 );
#endif

		clear_screen_flipper();
		SetTileTable(bg_tiles);

		for( p = 0 ; p < 23 ; p++ ) {
			bubbles[0][p] = random()%C_COUNT;
			bubbles[1][p] = random()%C_COUNT;
		}

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
			// Tile indices of arrow parts.
			sprites[SPRITE_ARROW+p].tileIndex = TILE_ARROW;
			sprites[SPRITE_RING +p].tileIndex = TILE_RING;
			sprites[SPRITE_RIVET+p].tileIndex = TILE_RIVET;

			firing[p] = false;	
			new_bubble(p); // Initialize next	
			new_bubble(p); // Initialise current and next
			draw_projectile(p);
		
			update_arrow(p);
			set_score( p, 0 );
		}
	
		drop = 0;
		wobble_timer = -WOBBLE_DELAY;
		game_over = false;
	
		SetSpriteVisibility(true);

		while(!game_over) {
			WaitVsync(1);

			for( p=0 ; p < players ; p++ ) {
				if( proc_controls(p) ) {
					update_arrow(p);
				}
				if( firing[p] ) {
					update_projectile(p);
				}
				if( players == 1 && ++wobble_timer > 0 ) {
					game_over = do_wobble();
				}
			}

			frame++;
		}
		
		if( players == 1 ) {
			DrawMap2( (SCREEN_TILES_H-FIELD_TILES_H)/2, FIELD_OFFSET_Y+(FIELD_TILES_V/2)-2, map_lose );
		}
		else {
		
		}
		
		while( ReadJoypad(0) == 0 && ReadJoypad(1) == 0 );
		while( ReadJoypad(0) != 0 || ReadJoypad(1) != 0 );
		SetSpriteVisibility(false);
		clear_screen_flipper();
	}
}

