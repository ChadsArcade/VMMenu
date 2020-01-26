#ifndef _VMMSTDDEF_H_
#define _VMMSTDDEF_H_

/*****************************************************************************
* Vector Mame Menu
*
* Author:  Chad Gray
* Created: 20/06/11
*
* Standard definitions
*
*****************************************************************************/

#ifndef _VCHARS_H_
	#include "vchars.h"
#endif

#define	FRAMES_PER_SEC 60			// number of frames drawn in a second
#define	EDGE_BRI			25			// Bright intensity
#define	EDGE_NRM			15			// Normal intensity
#define	EDGE_DIM			5			// Dim intensity
#define	BORD				5			// space between edge border frames
#define	NUM_ASTEROIDS	20			// number of asteroids on screensaver and exit screen
#define	NUM_STARS		40			// number of stars

/*** default key values ***/
#if defined(linux) || defined(__linux)
//SDL2 key codes:
	#define TILDE		0x0035			// Settings
	#define UP			0x0052			// Up
	#define DOWN		0x0051			// Down
	#define LEFT		0x0050			// (Rotate) Left
	#define RIGHT		0x004f			// (Rotate) Right
	#define ESC			0x0029			// Esc (1P Start + 2P Start on IPAC)
	#define FIRE		0x00e0			// Left CTRL key
	#define THRUST		0x00e2			// Left ALT key
	#define RSHIFT		0x003e			// Right Shift key
	#define LSHIFT		0x00e1			// Left Shift key
	#define HYPSPACE	0x002c			// Space bar
	#define CREDIT		0x0022			// 5 key
	#define START1		0x001e			// 1 key
	#define START2		0x001f			// 2 key

//SDL1.2 key codes:
//	#define TILDE		0x31			// Settings
//	#define UP			0x6f			// Up
//	#define DOWN		0x74			// Down
//	#define LEFT		0x71			// (Rotate) Left
//	#define RIGHT		0x72			// (Rotate) Right
//	#define ESC			0x09			// Esc (1P Start + 2P Start on IPAC)
//	#define FIRE		0x25			// Left CTRL key
//	#define THRUST		0x40			// Left ALT key
//	#define RSHIFT		0x3e			// Right Shift key
//	#define LSHIFT		0x32			// Left Shift key
//	#define HYPSPACE	0x41			// Space bar
//	#define CREDIT		0x0e			// 5 key
//	#define START1		0x0a			// 1 key
//	#define START2		0x0b			// 2 key

#else
	#define TILDE		0x2960		// Settings
	#define UP			0x4800		// Up
	#define DOWN		0x5000		// Down
	#define LEFT		0x4b00		// (Rotate) Left
	#define RIGHT		0x4d00		// (Rotate) Right
	#define ESC			0x011b		// Esc (1P Start + 2P Start on IPAC)
	#define FIRE		0x04			// Left CTRL key
	#define THRUST		0x08			// Left ALT key
	#define RSHIFT		0x01			// Right Shift key
	#define LSHIFT		0x02			// Left Shift key
	#define HYPSPACE	0x3920		// Space bar
	#define CREDIT		0x0635		// 5 key
	#define START1		0x0231		// 1 key
	#define START2		0x0332		// 2 key
#endif

//#define ROTATE		0x0534		// 4 key

/**vector object settings **/
#define	NewDir()			(((int)(rand()/(RAND_MAX/101)) < 50) ? -1 : 1)		// random -1 or +1
#define	NewXPos()		(int)((rand()/(RAND_MAX/(X_MAX-X_MIN)+1.0))-X_MAX)	// random between X_MIN and X_MAX
#define	NewYPos()		(int)((rand()/(RAND_MAX/(Y_MAX-Y_MIN)+1.0))-Y_MAX)	// random between Y_MIN and Y_MAX
#define	NewScale()		(int)((rand()/(RAND_MAX/3))+1)*2							// 2, 4, or 6
#define	NewXYInc()		(int)((rand()/(RAND_MAX/10))+1)							// random between 1 and 10
#define	NewTheta()		(int) (rand()/(RAND_MAX/5))								// random between 0 and 4
#define	NewAst()			(int) (rand()/(RAND_MAX/4))								// 0, 1, 2 or 3
#define	NewAstColour()	(int) (rand()/(RAND_MAX/7))								// 0 to 6
#define	NewStarSpeed()	(int)((rand()/(RAND_MAX/5))+1)							// 1 to 5

// Colours used in the menu
#define	vred		0
#define	vmagenta	1
#define	vcyan		2
#define	vblue		3
#define	vyellow	4
#define	vgreen	5
#define	vwhite	6

//LED codes
#if defined(linux) || defined(__linux)
	#define	S_LED 1			// Scroll lock
	#define	N_LED 16			// Number lock
	#define	C_LED 2			// Caps lock
#else
	#define	S_LED 1			// Scroll lock
	#define	N_LED 2			// Number lock
	#define	C_LED 4			// Caps lock
#endif 

// Index used to reference colour/intensity array
#define	c_col			0
#define	c_int			1
#define	c_glist		0
#define	c_sgame		1
#define	c_sman		2
#define	c_man			3
#define	c_pnman		4
#define	c_arrow		5
#define	c_asts		6

// Index of key codes
#define	k_menu		0
#define	k_options	1
#define	k_pman		2
#define	k_nman		3
#define	k_pgame		4
#define	k_ngame		5
#define	k_pclone		6
#define	k_nclone		7
#define	k_start		8
#define	k_quit		9
#define	k_random		10

// Index of user options
#define	o_rot			0
#define	o_stars		1
#define	o_ucase		2
#define	o_togpnm		3
#define	o_smenu		4
#define	o_redozvg	5
#define	o_mouse		6
#define	o_msens		7
#define	o_msamp		8
#define	o_mswapXY	9
#define	o_mrevX		10
#define	o_mrevY		11
#define	o_mpoint		12
#define	o_dovga		13
#define	o_attmode	14

typedef struct {
	float x, y;
} point;

/*******************************************************
Define a vector object, having:
	A vector shape									(outline)
	Position											(pos.x, pos.y)
	Velocity											(xinc, yinc)
	Scale												(xscale, yscale)
	An angle and rotation factor				(angle, theta)
	A point around which rotation occurs	(rot.x, rot.y)
	Brightness / colour							(red, green, blue)
	Edge behaviour (wrap or bounce)			(edge)
*******************************************************/
typedef struct {
	vShape	outline;
	point		pos;
	point		inc;
	point		scale;
	int		angle, theta;
	point		cent;
	int		colour;
	int		bright;
	int		edge;
} vObject;

/******************************************************
Define a vStar, having:
	position		pos.x, pos.y
	direction	delta.x, delta.y
	change		change.x, change.y
	speed			1-5
*******************************************************/
typedef struct {
	point		pos;
	point		delta;
	point		change;
	float		speed;
} vStar;

#endif

