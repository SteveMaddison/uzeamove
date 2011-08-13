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
#define BUBBLE_ODD_L_BLANK			13
#define BUBBLE_ODD_R_BLANK			14
// Number of bubble tiles of each colour
#define BUBBLES_PER_COLOUR			15
#define BUBBLE_SLIVER_L				9
#define BUBBLE_SLIVER_R				10

#define BUBBLE_WIDTH		12
#define FIELD_OFFSET_X		2
#define FIELD_OFFSET_Y		2
// Offset of player 2's field from that of player 1.
#define P2_TILE_OFFSET		13
#define P2_PIXEL_OFFSET		(TILE_WIDTH*P2_TILE_OFFSET)

#include "data/bg.inc"
#include "data/sprites.inc"

#define FIELD_BUBBLES_H		8
#define FIELD_BUBBLES_V		11
#define FIELD_TILES_H		12
#define FIELD_TILES_V		11
#define NUM_BUBBLES			83

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
#define SPRITE_RING			1
#define SPRITE_RIVET		2

#define SPRITE_CURRENT_L	3
#define SPRITE_CURRENT_R	4
#define SPRITE_PROJ_L		5
#define SPRITE_PROJ_R		6
#define SPRITE_NEXT_L		7
#define SPRITE_NEXT_R		8

#define SPRITES_PER_PLAYER	9

// Structures
typedef struct {
	char c; // Colour
	char angle;
	int x;
	int y;
} projectile_t;


// Globals
#define PLAYERS 1
unsigned char players = 1;
unsigned char bubbles[PLAYERS][NUM_BUBBLES];
unsigned char current[PLAYERS];
unsigned char next[PLAYERS];
char angle[PLAYERS];
projectile_t proj[PLAYERS];
bool block_fire[PLAYERS];
unsigned char block_left[PLAYERS];
unsigned char block_right[PLAYERS];
#define GEAR_ANIM_STEPS 2
char gear_anim[PLAYERS];
unsigned int frame = 0;


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
						else if( bubbles[player][b-1] != C_BLANK ) {
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
						else if( bubbles[player][b] != C_BLANK ) {
							if( bubbles[player][b-1] == C_BLANK ) {
								SetTile( xp, yp, BUBBLE_FIRST_COLOUR_TILE + (BUBBLES_PER_COLOUR*(bubbles[player][b]-1)) + BUBBLE_ODD_L_BLANK );
							}
							else {
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

void new_bubble( unsigned char player ) {
	unsigned char ps = (player*SPRITES_PER_PLAYER);
	
	current[player] = next[player];
	next[player] = (random()%(C_COUNT-1)) + 1;
	
	sprites[SPRITE_CURRENT_L+ps].tileIndex = TILE_BUBBLE_L( current[(int)player] );
	sprites[SPRITE_CURRENT_R+ps].tileIndex = TILE_BUBBLE_R( current[(int)player] );

	sprites[SPRITE_NEXT_L+ps].tileIndex = TILE_BUBBLE_L( next[(int)player] );
	sprites[SPRITE_NEXT_R+ps].tileIndex = TILE_BUBBLE_R( next[(int)player] );
}

bool proc_controls( unsigned char player ) {
	bool changed = false;
	
	if( block_left[player]  ) block_left[player]--;
	if( block_right[player] ) block_right[player]--;
	
	if( ReadJoypad(player) & BTN_LEFT ) {
		if( !block_left[player] ) {
			// Rotate left
			angle[(int)player]--;
			if( angle[(int)player] < -(ANGLES-1) ) {
				angle[(int)player] = -(ANGLES-1);
			}
			gear_anim[(int)player]--;
			if( gear_anim[(int)player] < 0 ) {
				gear_anim[(int)player] = GEAR_ANIM_STEPS-1;
			}
			changed = true;
			block_left[player] = 3;
			block_right[player] = 0;
		}
	}
	else {
		block_left[player] = 0;
	}
	
	if( ReadJoypad(player) & BTN_RIGHT ) {
		if( !block_right[player] ) {
			// Rotate right
			angle[(int)player]++;
			if( angle[(int)player] > ANGLES-1 ) {
				angle[(int)player] = ANGLES-1;
			}
			gear_anim[(int)player]++;
			if( gear_anim[(int)player] > GEAR_ANIM_STEPS-1 ) {
				gear_anim[(int)player] = 0;
			}
			changed = true;
			block_right[player] = 3;
			block_left[player] = 0;
		}
	}
	else {
		block_right[player] = 0;
	}
	
	if( ReadJoypad(player) & (BTN_A|BTN_B|BTN_X|BTN_Y) ) {
		if( !block_fire[player] ) {
			// Fire!
			unsigned char ps = (player*SPRITES_PER_PLAYER);

			proj[player].x = ((FIELD_TILES_H*TILE_WIDTH)/2) - (BUBBLE_WIDTH/2);
			proj[player].y = ((FIELD_TILES_V+1)*TILE_HEIGHT) - (BUBBLE_WIDTH/2);
			
			proj[player].x *= TRAJ_FACTOR;
			proj[player].y *= TRAJ_FACTOR;
			
			proj[player].c = current[player];
			proj[player].angle = angle[player];
		
			sprites[SPRITE_PROJ_L+ps].tileIndex = TILE_BUBBLE_L(proj[player].c);
			sprites[SPRITE_PROJ_R+ps].tileIndex = TILE_BUBBLE_R(proj[player].c);
		
			new_bubble(player);
		}
		block_fire[player] = true;
	}
	return changed;
}

void draw_arrow( unsigned char x, unsigned char y, unsigned char player ) {
	unsigned char ps = SPRITES_PER_PLAYER * player;
	if( angle[player] >= 0 ) {
		sprites[SPRITE_ARROW+ps].tileIndex = TILE_ARROW + ((angle[player]+2)/5);
	
		sprites[SPRITE_ARROW+ps].x = x + pgm_read_byte( arrow_x + angle[player] ) -4;
		sprites[SPRITE_RING+ps ].x = x + pgm_read_byte(  ring_x + angle[player] ) -4;
		sprites[SPRITE_RIVET+ps].x = x + pgm_read_byte( rivet_x + angle[player] ) -4;

		sprites[SPRITE_ARROW+ps].y = y - pgm_read_byte( arrow_y + angle[player] ) -2;
		sprites[SPRITE_RING+ps ].y = y - pgm_read_byte(  ring_y + angle[player] ) -5;
		sprites[SPRITE_RIVET+ps].y = y - pgm_read_byte( rivet_y + angle[player] ) -3;
	}
	else {
		sprites[SPRITE_ARROW+ps].tileIndex = TILE_ARROW + ((angle[player]-2)/5);

		sprites[SPRITE_ARROW+ps].x = x - pgm_read_byte( arrow_x - angle[player] ) -4;
		sprites[SPRITE_RING+ps ].x = x - pgm_read_byte(  ring_x - angle[player] ) -4;
		sprites[SPRITE_RIVET+ps].x = x - pgm_read_byte( rivet_x - angle[player] ) -4;

		sprites[SPRITE_ARROW+ps].y = y - pgm_read_byte( arrow_y - angle[player] ) -2;
		sprites[SPRITE_RING+ps ].y = y - pgm_read_byte(  ring_y - angle[player] ) -5;
		sprites[SPRITE_RIVET+ps].y = y - pgm_read_byte( rivet_y - angle[player] ) -3;
	}
}

void update_projectile( unsigned char player ) {
	unsigned char ps = (player*SPRITES_PER_PLAYER);

	proj[player].y -= pgm_read_byte( traj_y + proj[player].angle );
	
	if( proj[player].angle >= 0 ) {
		proj[player].x += pgm_read_byte( traj_x + proj[player].angle );
		if( proj[player].x >= ((FIELD_TILES_H*TILE_WIDTH)-BUBBLE_WIDTH) * TRAJ_FACTOR ) {
			// Handle bounce. Until we're back on the board, step back
			// through the last step we made. This calls for increased
			// precision...
			proj[player].x *= TRAJ_FACTOR;
			proj[player].y *= TRAJ_FACTOR;
			
			while( proj[player].x >= ((FIELD_TILES_H*TILE_WIDTH)-BUBBLE_WIDTH) * (TRAJ_FACTOR*TRAJ_FACTOR) ) {
				proj[player].x -= pgm_read_byte( traj_x + proj[player].angle );
				proj[player].y += pgm_read_byte( traj_y + proj[player].angle );
			}

			proj[player].x /= TRAJ_FACTOR;
			proj[player].y /= TRAJ_FACTOR;
			proj[player].angle = -proj[player].angle;
		}
	}
	else {
		proj[player].x -= pgm_read_byte( traj_x - proj[player].angle );
		while( proj[player].x < 0 ) {
			// Handle bounce
			proj[player].x *= TRAJ_FACTOR;
			proj[player].y *= TRAJ_FACTOR;
			
			while( proj[player].x < 0 ) {
				proj[player].x += pgm_read_byte( traj_x + proj[player].angle );
				proj[player].y += pgm_read_byte( traj_y + proj[player].angle );
			}

			proj[player].x /= TRAJ_FACTOR;
			proj[player].y /= TRAJ_FACTOR;
			proj[player].angle = -proj[player].angle;
		}
	}

	if( proj[player].y < 0 ) {
		proj[player].c = C_BLANK;
		sprites[SPRITE_PROJ_L+ps].tileIndex = 0;
		sprites[SPRITE_PROJ_R+ps].tileIndex = 0;
		block_fire[player] = false;
	}
	else {	
		if( players == 1 ) {
			sprites[SPRITE_PROJ_L+ps].x = ((SCREEN_TILES_H-FIELD_TILES_H)/2 * TILE_WIDTH) + (proj[player].x/TRAJ_FACTOR);
		}
		else {
			sprites[SPRITE_PROJ_L+ps].x = ((FIELD_OFFSET_X + (FIELD_TILES_H/2))*TILE_WIDTH)
				+ (P2_PIXEL_OFFSET*player) + (proj[player].x/TRAJ_FACTOR);
		}
		sprites[SPRITE_PROJ_R+ps].x = sprites[SPRITE_PROJ_L+ps].x + TILE_WIDTH;
	
		sprites[SPRITE_PROJ_L+ps].y = (FIELD_OFFSET_Y*TILE_HEIGHT) + (proj[player].y/TRAJ_FACTOR);
		sprites[SPRITE_PROJ_R+ps].y = sprites[SPRITE_PROJ_L+ps].y;
	}
}

int main(){
	int p;
	SetTileTable(bg_tiles);
	SetSpritesTileTable(sprite_tiles);
	ClearVram();

	for( p = 0 ; p < 16 ; p++ ) {
		bubbles[0][p] = random()%C_COUNT;
	}

	for( p=0 ; p<PLAYERS ; p++ ) {
		unsigned char ps = (p*SPRITES_PER_PLAYER);
		
		if( players == 1 ) {
			draw_field( (SCREEN_TILES_H-FIELD_TILES_H)/2, FIELD_OFFSET_Y, p );
			DrawMap2( (SCREEN_TILES_H-FIELD_TILES_H)/2, FIELD_OFFSET_Y+FIELD_TILES_V, map_panel );
		}
		else {
			draw_field( FIELD_OFFSET_X, FIELD_OFFSET_Y, p );
			DrawMap2( FIELD_OFFSET_X, FIELD_OFFSET_Y+FIELD_TILES_V, map_panel );
	
			draw_field( SCREEN_TILES_H - FIELD_TILES_H - FIELD_OFFSET_X -1, FIELD_OFFSET_Y, 1 );
			DrawMap2( SCREEN_TILES_H - FIELD_TILES_H - FIELD_OFFSET_X -1, FIELD_OFFSET_Y+FIELD_TILES_V, map_panel );
		}
	
		// Tile indeces of arrow parts.
		sprites[SPRITE_ARROW+ps].tileIndex = TILE_ARROW;
		sprites[SPRITE_RING +ps].tileIndex = TILE_RING;
		sprites[SPRITE_RIVET+ps].tileIndex = TILE_RIVET;

		// Position and indeces of current/next bubbles.
		if( players == 1 ) {
			sprites[SPRITE_CURRENT_L+ps].x = ((SCREEN_TILES_H/2) * TILE_WIDTH) - (BUBBLE_WIDTH/2);
		}
		else {
			sprites[SPRITE_CURRENT_L+ps].x = ((FIELD_OFFSET_X + (FIELD_TILES_H/2))*TILE_WIDTH) + (P2_PIXEL_OFFSET*p) - (BUBBLE_WIDTH/2);
		}
		
		sprites[SPRITE_CURRENT_L+ps].y = ((FIELD_OFFSET_Y+FIELD_TILES_V)*TILE_HEIGHT) + (BUBBLE_WIDTH/2);
		sprites[SPRITE_NEXT_L+ps].x = sprites[SPRITE_CURRENT_L+ps].x + 40;
		sprites[SPRITE_NEXT_L+ps].y = sprites[SPRITE_CURRENT_L+ps].y + 1;
		
		sprites[SPRITE_CURRENT_R+ps].x = sprites[SPRITE_CURRENT_L+ps].x + TILE_WIDTH;
		sprites[SPRITE_CURRENT_R+ps].y = sprites[SPRITE_CURRENT_L+ps].y;
		sprites[SPRITE_NEXT_R+ps].x = sprites[SPRITE_NEXT_L+ps].x + TILE_WIDTH;
		sprites[SPRITE_NEXT_R+ps].y = sprites[SPRITE_NEXT_L+ps].y;
		
		new_bubble(p); // Initialise next
		new_bubble(p); // Initialise current and next
		proj[p].c = C_BLANK;
	}

	while(1) {
		WaitVsync(1);

		for( p=0 ; p < players ; p++ ) {
			if( proc_controls(p) || frame == 0 ) {
				if( players == 1 ) {
					draw_arrow(
						(SCREEN_TILES_H/2) * TILE_WIDTH,
						((FIELD_OFFSET_Y + FIELD_TILES_H)*TILE_HEIGHT), p );

					if( gear_anim[p] == 0 ) {
						DrawMap2( ((SCREEN_TILES_H-FIELD_TILES_H)/2)+2 , FIELD_OFFSET_Y+FIELD_TILES_V, map_gears1 );
					}
					else {
						DrawMap2( ((SCREEN_TILES_H-FIELD_TILES_H)/2)+2, FIELD_OFFSET_Y+FIELD_TILES_V, map_gears2 );
					}
				}
				else {
					draw_arrow(
						((FIELD_OFFSET_X + (FIELD_TILES_H/2))*TILE_WIDTH) + (P2_PIXEL_OFFSET*p),
						(FIELD_OFFSET_Y + FIELD_TILES_H) * TILE_HEIGHT, p );

					if( gear_anim[p] == 0 ) {
						DrawMap2( FIELD_OFFSET_X+2 + (P2_TILE_OFFSET*p), FIELD_OFFSET_Y+FIELD_TILES_V, map_gears1 );
					}
					else {
						DrawMap2( FIELD_OFFSET_X+2 + (P2_TILE_OFFSET*p), FIELD_OFFSET_Y+FIELD_TILES_V, map_gears2 );
					}
				}
			}
			if( proj[p].c != C_BLANK ) {
				update_projectile(p);
			}
		}

		frame++;
	}
}

