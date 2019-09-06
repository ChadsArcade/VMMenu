/*****************************************************************************
* Vector Mame Menu
*
* Author:  Chad Gray
* Created: 10/11/09 - 22/06/11
*
* History:
*
* 22/6/11	Ported to SDL/Linux and pulled out common source
* 13/3/11	Added keyboard LEDs
*			Added Mouse/Trackball support
*			Press a key for menu message in screen saver
* 12/3/11	Added Spinner Support
*
*
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "zstddef.h"
#include "zvgPort.h"
#include "zvgEnc.h"
#include "timer.h"
#include "zvgFrame.h"
#include "gamelist.h"
#include "vchars.h"
#include "iniparser.h"
#include <time.h>
#include "vmmstddef.h"

//OS Specific Headers
#if defined(linux) || defined(__linux)
	#include "LinuxVMM.h"
#else
	#include "DOSvmm.h"
#endif 

/****Function declarations***/
void		PrintString(char*, int, int, int, float, float, int);			// prints a string of characters
point		fnrotate(int, float, float, float, float);						// rotate a point given number of degrees
void		drawshape(vObject);												// draw shape pointed to by vObject
vObject		updateobject(vObject);											// update position and rotation of a vector object
void		drawborders(int, int, int, int, int, int, int);					// draw borders around edge of screen
char*		ucase(char*);													// convert string to uppercase
vObject		intro(void);													// Intro logo animation
vObject		make_asteroid(void);											// define an asteroid
vObject		make_sega(void);												// define sega logo
vObject		make_cinematronics(void);										// define cinematronics logo
vObject		make_atari(void);												// define atari logo
vObject		make_centuri(void);												// define centuri logo
vObject		make_vbeam(void);												// define vectorbeam logo
vObject		make_midway(void);												// define midway logo
int			reallyescape(void);												// See if you really want to quit
void		author(int);													// Print author info
void		setcolour(int, int);											// set colour and brightness
int			credits(void);													// print credits
vStar		make_star(void);												// define a star
vStar		updatestar(vStar);												// update a star object
void		drawstar(vStar);												// Draw a star
void		showstars();													// Display all the stars on screen
void		getsettings(void);												// get settings from ini file
void		writeinival(char*, int, int, int);								// write a value to the cfg file
void		writecfg(void);													// write the cfg file
int			getcolour(char*);												// Get colour value from cfg as an int
void		pressakey(int, int);											// Message to escape from screen saver to menu
void		GetRGBfromColour(int, int*, int*, int*);						// Get R, G and B components of a passed colour
g_node*		GetRandomGame(m_node *);										// Selects a random game from the list
void		PlayAttractGame(m_node *gameslist);								// get a random game name and add attract mode args
void		PrintPointer(int mx, int my);									// Print mouse pointer at current mouse position
void		SetOptions(void);

//#define	DEBUG							// comment out to suppress debug output

// Global variables (sorry - there's quite a few...)

vObject		asteroid[NUM_ASTEROIDS];
vStar		starz[NUM_STARS];
static int	xmax=X_MAX, ymax=Y_MAX;

extern int	optz[15];						// array of user defined menu preferences
static int	keyz[11];						// array of key press codes
static int	colours[2][7];					// array of [colours][7] and [intensities][7]

static char	attractargs[30];
extern char	zvgargs[30];
static int	totalnumgames=0;

static dictionary* ini;

extern int 	mousexmick, mouseymick;
int	ZVGPresent = 1;
int	SDL_VC, SDL_VB;							// colour and brightness for SDL vectors
int	mousefound=0;

char	auth1[] = "VMMenu 1.3, Chad Gray 2011";
char	auth2[] = "ChadsArcade@Gmail.com";

int main( void) //int argc, char *argv[])
{
	uint	err, man_menu;
	int		count, top, timeout = 0, ticks = 0, gamesize;
	int		pressx=0, pressy=0;
	int		cc;
	float	width=0.0;
	char	mytext[100];
	int		mpx = 0, mpy = 0;
	vObject	mame, sega, cinematronics, atari, centuri, vbeam, midway;
	m_node	*vectorgames;
	g_node	*gamelist_root = NULL, *sel_game = NULL, *sel_clone = NULL;
	FILE	*inifp;
	char	* ini_name = "vmmenu.cfg";

	vectorgames = createlist();
	totalnumgames=printlist(vectorgames);
	linklist(vectorgames);
	sel_game = vectorgames->firstgame;
	sel_clone = sel_game;
	man_menu = zTrue;

	inifp = fopen (ini_name, "r" );
	if (inifp != NULL)
	{
		fclose(inifp);
		ini = iniparser_load(ini_name);
		if (ini == NULL)
		{
			exit(0);							// return to OS
		}
	}
	else
	{
		ini = dictionary_new(0);
		printf("Creating new CFG file.\n");
	}
	getsettings();
	// Set up rotation
	if ((optz[o_rot] == 1) || (optz[o_rot] == 3))
	{
		xmax = Y_MAX;
		ymax = X_MAX;
	}
	else
	{
		xmax = X_MAX;
		ymax = Y_MAX;
	}

	startZVG();
	srandom(time(NULL));
	setLEDs(0);

	//printf("o_mouse: %d o_mpoint: %d\n", optz[o_mouse], optz[o_mpoint]);
	// Set up mouse
	mousefound = initmouse();
	if (optz[o_mouse] || optz[o_mpoint])
	{
		if (mousefound)
		{
			if (optz[o_mouse] == 1 && !optz[o_mpoint])
			{
				printf("\n1-axis (Spinner) control activated.\n");
			}
			else
			{
				printf("\n2-axis (Mouse/Trackball) control activated.\n");
			}
		}
		else
		{
			mousefound = 0;
			printf("\nMouse/Spinner not found - device disabled.\n");
		}
	}
	else printf("No mouse configured.\n");

#ifdef DEBUG
	//print out a ZVG banner, indicating version etc.
	zvgBanner( ZvgSpeeds, &ZvgID);
	printf("\n\n");
#endif

	// define 20 asteroids to randomly move around screen
	for (count=0; count < NUM_ASTEROIDS; count++)
	{
		asteroid[count] = make_asteroid();
	}
	for (count=0; count < NUM_STARS; count++)
	{
		starz[count] = make_star();
	}

	sega			= make_sega();
	cinematronics	= make_cinematronics();
	atari			= make_atari();
	centuri			= make_centuri();
	vbeam			= make_vbeam();
	midway			= make_midway();

	if (ZVGPresent)
	{
		tmrSetFrameRate( FRAMES_PER_SEC);
		zvgFrameSetClipWin( X_MIN, Y_MIN, X_MAX, Y_MAX);
	}

	/*** At this point we have a blank screen. Run an intro with the mame logo ***/
	mame = intro();

	// Start main loop
	while (1)
	{
		// Read mouse
		//if (mousefound && optz[o_mouse] && (timeout%optz[o_msamp] == 0))
		if (mousefound && (timeout%optz[o_msamp] == 0))	// mousefound only set if optz[o_mouse] is true
		{
			mousemick();			 		// update mousexmick and mouseymick
		}
		else
		{
			mousexmick = 0;
			mouseymick = 0;
		}

		if (optz[o_stars]) showstars();
		cc=getkey();						// Check keys

		if (timeout > 1800)		// ############## screensaver mode 1800 * 1/60 = 30 seconds ##############
		{
			//if ((ticks%60 == 5) || (ticks%60 == 35)) setLEDs(ticks%60 <30 ? N_LED : C_LED); // flash LEDs
			if ((ticks%360) == 0)	setLEDs(0);
			if ((ticks%360) == 60)  setLEDs(S_LED);
			if ((ticks%360) == 120) setLEDs(S_LED | N_LED);
			if ((ticks%360) == 180) setLEDs(S_LED | N_LED | C_LED);
			if ((ticks%360) == 240) setLEDs(N_LED | C_LED);
			if ((ticks%360) == 300) setLEDs(C_LED);

			for (count=0; count < NUM_ASTEROIDS; count++)
			{
				drawshape(asteroid[count]);
				asteroid[count] = updateobject(asteroid[count]);
			}
			drawshape(mame);
			mame = updateobject(mame);
			if ((timeout % 300) == 100)
			{
				pressx = NewXPos();
				pressy = NewYPos();
			}
			if ((timeout % 300) > 150)		pressakey(pressx, pressy);
			if ((timeout % 1800) > 1500)
				author(37.5-(abs(((timeout % 1800)-1650)/4)));
			if (cc || (abs(mousexmick) > optz[o_msens]))							// if you've hit a key exit screensaver
			{
				timeout = 0;
				mame.inc.x = ((NewXYInc() * NewDir() ) / 4) + 0.25;	// choose a new angle for mame logo to move in
				mame.inc.y = ((NewXYInc() * NewDir() ) / 4) + 0.25;	// in case the last one wasn't very good :-)
				for (count=0; count < NUM_ASTEROIDS; count++)			// Re-randomize the asteroids too
				{
					asteroid[count] = make_asteroid();
				}
			}
			if ((timeout%18000 == 0) && (timeout > 0))								// show credits every 5 mins of screensaver time
			{
				if (credits()>0) timeout=0;
				else timeout++;
			}
 			if ((timeout%4500 == 0) && (timeout > 900) && optz[o_attmode])			// show random game every 1:15
			{
				PlayAttractGame(vectorgames);
			}
		}
		else							// ############## menu mode ##############
		{
			// mouse pointer
			if (mousefound && optz[o_mpoint])
			{
				mousepos(&mpx, &mpy);
				PrintPointer(mpx, mpy);
			}
			// keyboard led flash if in game menu
			if (!man_menu)
			{
				if ((ticks%60 == 5) || (ticks%60 == 35)) setLEDs(ticks%60 <30 ? 0 : N_LED);
			}
			else	setLEDs(0);
			if (cc || (abs(mousexmick) > optz[o_msens]) || (abs(mouseymick) > optz[o_msens]))	// if you've hit a key process it...
			{
				timeout = 0;																	// reset screensaver timer
				/*** Keys when in any menu ***/
				if (cc == keyz[k_options]) SetOptions();
				if (cc == keyz[k_menu])			man_menu = !man_menu;	// Toggle between manufacturer and game menus
				if (cc == keyz[k_random]) RunGame(GetRandomGame(vectorgames)->clone);
				if (cc == keyz[k_quit])
				{
					if (reallyescape()) break;													// if ESC confirmed, exit menu
				}

				// ############## Keys when in manufacturer menu ##############
				if (man_menu)
				{
					if ((cc == keyz[k_pman]) || (mousexmick < -optz[o_msens]))
					{
						vectorgames = vectorgames->pmanuf;
						sel_game = vectorgames->firstgame;
						sel_clone = sel_game;
					}
					if ((cc == keyz[k_nman]) || (mousexmick > optz[o_msens]))
					{
						vectorgames = vectorgames->nmanuf;
						sel_game = vectorgames->firstgame;
						sel_clone = sel_game;
					}
					if (optz[o_smenu])
					{
						if ((cc == keyz[k_ngame]) || ((mouseymick > optz[o_msens])))
						{
							sel_game = vectorgames->firstgame;
							sel_clone = sel_game;
							man_menu = 0;
							cc = 0;
						}
						if ((cc == keyz[k_pgame]) || ((mouseymick < -optz[o_msens])))
						{
							sel_game = vectorgames->firstgame->prev;
							sel_clone = sel_game;
							man_menu = 0;
							cc = 0;
						}
					}
				}

				// ############## Keys when in game menu ##############
				else
				{
					if (optz[o_smenu])
					{
						if (((cc == keyz[k_pgame]) || (mouseymick < -optz[o_msens])) && (sel_game == vectorgames->firstgame))
						{
							man_menu = 1;
							cc = 0;
						}
						if (((cc == keyz[k_ngame]) || (mouseymick > optz[o_msens])) && (sel_game == vectorgames->firstgame->prev))
						{
							man_menu = 1;
							cc = 0;
						}
					}
					if ((cc == keyz[k_pgame]) || ((mousexmick < -optz[o_msens]) && (optz[o_mouse]==1)) || ((mouseymick < -optz[o_msens])))	// go to previous game
					{
						sel_game = sel_game->prev;
						sel_clone = sel_game;
					}
					if ((cc == keyz[k_ngame]) || ((mousexmick > optz[o_msens]) && (optz[o_mouse] ==1)) || ((mouseymick > optz[o_msens])))	// go to next game
					{
						sel_game = sel_game->next;
						sel_clone = sel_game;
					}
					if (cc == keyz[k_start])				// launch VMAME
					{
						RunGame(sel_clone->clone);
					}
					if ((cc == keyz[k_nclone]) || (mousexmick > optz[o_msens] && (optz[o_mouse] == 2)))		// go to next clone
					{
						if		(sel_clone->nclone == NULL) sel_clone = sel_game;
						else	sel_clone = sel_clone->nclone;
					}
					if ((cc == keyz[k_pclone]) || (mousexmick < -optz[o_msens] && (optz[o_mouse] == 2)))	// go to previous clone
					{
						if		(sel_clone == sel_game) sel_clone = gotolastclone(sel_game);
						else	sel_clone = sel_clone->pclone;
						if (sel_clone == NULL)	sel_clone = sel_game;
					}
				}
			}

			drawborders(-X_MAX, -Y_MAX, X_MAX, Y_MAX, 0, 3, vwhite);						// Draw frame around the edge of the screen

			// print the manufacturer name
			strcpy(mytext, vectorgames->name);
			if (man_menu)
				setcolour(colours[c_col][c_sman], colours[c_int][c_sman]);
			else
				setcolour(colours[c_col][c_man], colours[c_int][c_man]);
			if (!optz[o_togpnm]) PrintString(mytext, 0, 300, 0, 12, 12, 0);					// manufacturer name
			else PrintString(mytext, 0, 300, 0, 8+(4*(1-man_menu)), 8+(4*(1-man_menu)), 0);

			setcolour(colours[c_col][c_arrow], colours[c_int][c_arrow]);
			if (man_menu && optz[o_smenu])
				PrintString(">", 0, 220, 270, 6, 6, 0);
			if (!man_menu && optz[o_smenu])
				PrintString(">", 0, 210, 90, 6, 6, 0);

			// print the next and previous manufacturers at the sides and arrows
			if (man_menu)
			{
				setcolour(colours[c_col][c_arrow], colours[c_int][c_arrow]);
				PrintString("<", -((strlen(mytext)*(36-(12*optz[o_togpnm])))/2) - 40, 300, 0, 7, 7, 0);
				PrintString(">", ((strlen(mytext)*(36-(12*optz[o_togpnm])))/2) + 20, 300, 0, 7, 7, 0);

				if (optz[o_togpnm])
				{
					setcolour(colours[c_col][c_pnman], colours[c_int][c_pnman]);
					strcpy(mytext, vectorgames->pmanuf->name);			// Print previous manufacturer name
					PrintString(mytext, -(xmax-((strlen(mytext)*7)+40)), 300, 0, 6, 6, 0);
					strcpy(mytext, vectorgames->nmanuf->name);			// print next manufacturer name
					PrintString(mytext, xmax-((strlen(mytext)*7)+40), 300, 0, 6, 6, 0);
				}
			}

			/*** Print manufacturer logos at side of screen ***/
			strcpy(mytext, vectorgames->name);
			ucase(mytext);
			if (ticks > 240) width = cos(((ticks-240)*3)*M_PI/180); // 2 sec rotation (120 frames) so we mult by 3 for 360 degrees
			else width = 1;
			if (!strcmp(mytext, "SEGA"))
			{
				sega.pos.x = -(xmax-80);
				sega.angle = 90;
				sega.scale.y = width;
				drawshape(sega);
				sega.pos.x = xmax-80;
				sega.angle = 270;
				drawshape(sega);
			}
			else if (!strcmp(mytext, "CENTURI"))
			{
				centuri.pos.x = -(xmax-80);
				centuri.angle = 90;
				centuri.scale.y = width;
				drawshape(centuri);
				centuri.pos.x = xmax-80;
				centuri.angle = 270;
				drawshape(centuri);
			}
			else if (!strcmp(mytext, "CINEMATRONICS"))
			{
				cinematronics.pos.x = -(xmax-130);
				cinematronics.scale.x = width;
				drawshape(cinematronics);
				cinematronics.pos.x = xmax-130;
				drawshape(cinematronics);
			}
			else if (!strcmp(mytext, "MIDWAY"))
			{
				midway.pos.x = -(xmax-130);
				midway.scale.x = width;
				drawshape(midway);
				midway.pos.x = xmax-130;
				drawshape(midway);
			}
			else if (!strcmp(mytext, "ATARI"))
			{
				atari.pos.x = -(xmax-130);
				atari.scale.x = width;
				drawshape(atari);
				atari.pos.x = xmax-130;
				drawshape(atari);
			}
			else if (!strcmp(mytext, "VECTORBEAM"))
			{
				vbeam.pos.x = -(xmax-80);
				vbeam.angle = 90;
				vbeam.scale.y = width;
				drawshape(vbeam);
				vbeam.angle = 270;
				vbeam.pos.x = xmax-80;
				drawshape(vbeam);
			}
			else
			{
				strcpy(mytext, vectorgames->name);			// print manufacturer name in text
				setcolour(vcyan, EDGE_BRI-4);
				PrintString(mytext, -(xmax-80), 0, 90, 14, width*14, 90);
				PrintString(mytext, xmax-80, 0, 270, 14, width*14, 270);
				setcolour(vcyan, EDGE_BRI-8);
				PrintString(mytext, -(xmax-82), 2, 90, 14, width*14, 90);
				PrintString(mytext, xmax-82, 2, 270, 14, width*14, 270);
				setcolour(vcyan, EDGE_BRI-12);
				PrintString(mytext, -(xmax-84), 4, 90, 14, width*14, 90);
				PrintString(mytext, xmax-84, 4, 270, 14, width*14, 270);
			}

			// Now print the games list menu
			top = 150;
			gamelist_root = vectorgames->firstgame;										// point to game list for current manufacturer
			do
			{
				strcpy(mytext, gamelist_root->name);									// mytext = name of parent game
				gamesize = 4;																		// fontsize for gamelist
				if (!man_menu && (sel_game == gamelist_root))							// if we're at the selected game...
				{
					gamesize = 5;
					if (sel_game != sel_clone) strcpy(mytext, sel_clone->name);			// change to clone name if different
					if (sel_game->nclone)
					{
						setcolour(colours[c_col][c_arrow], colours[c_int][c_arrow]);
						PrintString(">", ((strlen(mytext)*3*gamesize)/2) + 3*gamesize, top, 0, gamesize-1, gamesize-1, 0);
						PrintString("<", -((strlen(mytext)*3*gamesize)/2) - 6*gamesize, top, 0, gamesize-1, gamesize-1, 0);
					}
					setcolour(colours[c_col][c_sgame], colours[c_int][c_sgame]);
				}
				else
				{
					setcolour(colours[c_col][c_glist], colours[c_int][c_glist]);
					if (strstr(mytext, " (") != NULL)
						mytext[strstr(mytext, " (") - mytext] = 0;						// ... and strip off version info
				}
				PrintString(mytext, 0, top, 0, gamesize, gamesize, 0);
				gamelist_root = gamelist_root->next;
				top -= 35;
			}
			while (gamelist_root != vectorgames->firstgame);
		}
		timeout ++;													// screensaver timer
		ticks=(ticks+1)%360;										// counter
		err = sendframe();
		if (err) break;
	}

	// if loop exited, save config and return to DOS
	inifp = fopen (ini_name, "w" );
	if (inifp != NULL)
	{
		printf("Writing cfg file... ");
		writecfg();
		iniparser_dump_ini(ini, inifp);
		printf("Done.\n");
		fclose(inifp);
	}
	iniparser_freedict(ini);
	printf("Quitting to OS...\n");
	cc=credits();
	printf("\n%s, (c) 2009-2011\n", auth1);
	ShutdownAll();
	return (0);
}


/*******************************************************************
 Print a string of text on screen centred around position x,y
 Scale factor x,y
 Characters rotated by charangle
 along a line rotated by lineangle
********************************************************************/
void PrintString(char *text, int xpos, int ypos, int charangle, float xScale, float yScale, int lineangle)
{
	int ii, vv, length;
	int x1, y1, x2, y2, width = 2;
	float  x_delta, y_delta, textx, texty, halfstring;
	point line_start, line_end, cvs, cve;
	vShape printchar;
	xScale = xScale/(width/2);

	// calculate halfway point of string to centre it around given x position
	length = strlen(text);
	halfstring = (length * (width + 1) * xScale) / 2;		// 3 = char width + 1px space

	// Calculate the start and endpoints of the line we are writing on
	// and then rotate the line about the centre point.
	line_start	= fnrotate(lineangle, xpos - halfstring, ypos, xpos, ypos);
	line_end	= fnrotate(lineangle, xpos + halfstring, ypos, xpos, ypos);

	// The difference between start and end X and Y values divided by the number of chars
	// gives us the increment in x and y to the print position of the next character
	x_delta = (line_end.x - line_start.x) / length;
	y_delta = (line_end.y - line_start.y) / length;

	// Set the character print position to the beginning of the line.
	// Adjust X/Y else the first char's centre is on the endpoint
	textx = line_start.x + (x_delta / 2);
	texty = line_start.y + (y_delta / 2);

#ifdef DEBUG
	drawvector(line_start, line_end, 0, 0);			// For testing, draw the line the text will be printed along
#endif

	// loop for each character in the string
	for (ii=0;ii<length;ii++)
	{
		if (optz[o_ucase]) printchar = fnGetChar(toupper(text[ii]));
		else printchar = fnGetChar(text[ii]);
		
		//Loop for each vector in the character
		for (vv=0;vv<printchar.size;vv+=4)
		{
			x1 = printchar.array[vv+0] -(width/2);	// -1 to move origin to centre of char (total width = 2)
			y1 = printchar.array[vv+1] -2;	// -2 to move origin to centre of char (total height = 4)
			x2 = printchar.array[vv+2] -(width/2);	// This positions the characters through the centre of the line
			y2 = printchar.array[vv+3] -2;	// rather than *on* it which makes rotation work correctly
			cvs = fnrotate(charangle, x1 * xScale, y1 * yScale, 0, 0);
			cve = fnrotate(charangle, x2 * xScale, y2 * yScale, 0, 0);
			drawvector(cvs, cve, textx, texty);
		}
		textx += x_delta;
		texty += y_delta;
	}
}


/*******************************************************************
 Rotate co-ordinates x,y around the origin. Returns a point.
 trans x,y provides offset to translate rotation point to origin
********************************************************************/
point fnrotate(int angle, float x, float y, float x_trans, float y_trans)
{
	point p;
	x = (x - x_trans);
	y = (y - y_trans);
	p.x = (x * cos(angle * (M_PI/180)) - y * sin(angle * (M_PI/180))) + x_trans;
	p.y = (x * sin(angle * (M_PI/180)) + y * cos(angle * (M_PI/180))) + y_trans;
	return p;
}


/*******************************************************************
 Draw a shape at position x, y on screen, rotated by 'angle' degrees
 and scaled by factors xScale, yScale
 centre x,y provides point about which to rotate object
********************************************************************/
void drawshape(vObject shape)
{
	int ii;
	float x1, x2, y1, y2;
	point start_p, end_p;
	setcolour(shape.colour, shape.bright);
	for (ii=0; ii<shape.outline.size; ii+=4)
	{
		x1 = shape.outline.array[ii + 0] - shape.cent.x;
		y1 = shape.outline.array[ii + 1] - shape.cent.y;
		x2 = shape.outline.array[ii + 2] - shape.cent.x;
		y2 = shape.outline.array[ii + 3] - shape.cent.y;
		// This next line is a horrible hack to change colour to red when the
		// co-ords of "vector" in the Mame logo are found
		// I did say colour was an afterthought... one day I'll implement
		// a tandem colour array for each vector in a vObject
		if ((shape.outline.array[ii + 0] == 154) && (shape.outline.array[ii + 1] == 20) && (shape.outline.array[ii + 2] == 154) && (shape.outline.array[ii + 3] == 0)) setcolour(vred, shape.bright);
		start_p	= fnrotate(shape.angle, x1 * shape.scale.x, y1 * shape.scale.y, 0, 0);
		end_p		= fnrotate(shape.angle, x2 * shape.scale.x, y2 * shape.scale.y, 0, 0);
		//printf("x: %f, y:%f\n", start_p.x, start_p.y);
		drawvector(start_p, end_p, shape.pos.x, shape.pos.y);
	}
}


/*******************************************************************
				Draw borders between x and y values
********************************************************************/
void drawborders(int x1, int y1, int x2, int y2, int rot, int frames, int colour)
{
	int	c, rotvalue;
	point p1, p2, p3, p4;
	rotvalue = optz[o_rot];
	if (!rot) optz[o_rot] = 0;
	p1.x = x1;
	p1.y = y1;
	p2.x = x2;
	p2.y = y2;
	for (c=1; c<(frames+1); c++)
	{
		setcolour(colour, EDGE_BRI-(c*5));
		p3.x = p1.x;
		p3.y = p2.y;
		p4.x = p2.x;
		p4.y = p1.y;

		drawvector(p1, p3, 0, 0);
		drawvector(p3, p2, 0, 0);
		drawvector(p2, p4, 0, 0);
		drawvector(p4, p1, 0, 0);

		p1.x += BORD;
		p1.y += BORD;
		p2.x -= BORD;
		p2.y -= BORD;
	}
	if (frames == 1)			// dropshadow, just for the beans of it
	{
		p2.x = p4.x + 20;
		p2.y = p3.y - 20;
		p1.x = p2.x;
		p1.y = p3.y;
		drawvector(p1, p2, 0, 0);
		p1.x = p3.x + 20;
		p1.y = p2.y;
		drawvector(p1, p2, 0, 0);
		p2.x = p1.x;
		p2.y = p4.y - 20;
		drawvector(p1, p2, 0, 0);
		p1.x = p3.x;
		p1.y = p2.y;
		drawvector(p1, p2, 0, 0);
	}
	optz[o_rot] = rotvalue;
}


/********************************************************************
Update a vector object:
	* increment x position and wrap or bounce if boundary hit
	* increment y position and wrap or bounce if boundary hit
	* increment angle
********************************************************************/
vObject updateobject(vObject shape)
{
	if (shape.inc.x != 0)
	{
		shape.pos.x += shape.inc.x;
		switch (shape.edge)
		{
		case 1:			// wrap around edge
			if (shape.pos.x < -xmax) shape.pos.x = xmax;
			if (shape.pos.x > xmax) shape.pos.x = -xmax;
			break;
		case -1:		// bounce at edge
			if ((shape.pos.x + shape.cent.x) > xmax || (shape.pos.x - shape.cent.x) < -xmax)
				shape.inc.x = -shape.inc.x;
			break;
		default:		// stop at edge
			if (shape.pos.x > xmax || shape.pos.x < -xmax) shape.inc.x = 0;
			break;
		}
	}
	if (shape.inc.y != 0)
	{
		shape.pos.y += shape.inc.y;
		switch (shape.edge)
		{
		case 1:			// wrap around edge
			if (shape.pos.y < -ymax) shape.pos.y = ymax;
			if (shape.pos.y > ymax) shape.pos.y = -ymax;
			break;
		case -1:		// bounce at edge
			if ((shape.pos.y + shape.cent.y) > ymax || (shape.pos.y - shape.cent.y) < -ymax)
				shape.inc.y = -shape.inc.y;
			break;
		default:		// stop at edge
			if (shape.pos.y > ymax || shape.pos.y < -ymax) shape.inc.y = 0;
			break;
		}
	}
	shape.angle = (shape.angle + shape.theta) % 360;	// increment angle
	return shape;
}


/*************************************
		Convert string to uppercase
**************************************/
char* ucase(char *str)
{
	int i;
	for( i = 0; str[ i ]; i++)
		str[ i ] = toupper( str[ i ] );
	return str;
}


/***********************************
		Create mame logo intro
************************************/
vObject intro(void)
{
	vObject	mame;
	int		count;
	float		bright = EDGE_NRM;

	// x from 0 to 390	(3rd vector of E) centre 195
	// y from 0 to 109	(1st vector of A) centre 55

	static int mamelogo[] = {
	0,27,81,108,		81,108,81,73,		81,73,117,109,		117,109,117,51,							// M
	117,51,174,109,		174,109,174,44,																	// A
	174,44,239,109,		239,109,239,73,		239,73,275,109,		275,109,275,54,							// M
	275,54,328,107,		328,107,390,107,	390,107,371,88,		371,88,340,88,		340,88,330,78,		// E
	330,78,346,78,		346,78,325,57,		325,57,310,57,		310,57,299,47,		299,47,353,47,		// L
	353,47,333,27,		333,27,249,27,		249,27,254,33,												// O
	254,33,254,55,		254,55,224,25,		224,25,224,60,		224,60,190,27,		190,27,154,27,		// G
	154,27,154,55,		154,55,98,0,		98,0,65,0,			65,0,96,31,								// O
	96,31,96,55,		96,55,66,25,		66,25,66,60,		66,60,32,27,		32,27,0,27,			// 
	154,20,154,0,		154,0,174,20,																	// V
	198,20,183,20,		183,20,163,0,		163,0,183,0,		173,10,183,10,							// E
	223,20,208,20,		208,20,188,0,		188,0,208,0,												// C
	228,20,248,20,		238,20,218,0,																	// T
	253,20,268,20,		268,20,248,0,		248,0,233,0,		233,0,253,20,							// O
	253,0,273,20,		273,20,288,20,		288,20,278,10,		278,10,273,10,		273,10,273,0		// R
	};

	mame.outline.array = mamelogo;
	mame.outline.size = sizeof(mamelogo) / sizeof(*mamelogo);
	mame.pos.x = 0;			// centre screen
	mame.pos.y = 0;			// centre screen
	mame.inc.x = 0;			// don't move
	mame.inc.y = 0;			// don't move
	mame.angle = 0;			// normal orientation
	mame.theta = -3;		// rotate clockwise 3 degrees per increment
	mame.cent.x = 195;		// defines centre of logo
	mame.cent.y = 55;		// centre of logo (rotation origin)
	mame.colour = vcyan;
	mame.bright = EDGE_NRM;
	mame.edge  = -1;		// bounce on edge contact
	mame.scale.x = 0.01;	//starting scale factor
	mame.scale.y = mame.scale.x;

	// Zoom from 0 to x1.5 whilst rotating clockwise through 720 degrees
	for (count=0;count<120;count++)
	{
		drawshape(mame);
		mame = updateobject(mame);
		mame.scale.x += (1.5/120);
		mame.scale.y = mame.scale.x;
		author(EDGE_BRI);
		if (optz[o_stars]) showstars();
		sendframe();
	}

	mame.angle = 0;			// set to normal orientation
	mame.theta = 0;			// no rotation this time

	// Brighten from NRM to BRI over 1 second
	for (count=0;count<60;count++)
	{
		bright += 0.5;
		mame.bright = bright;
		drawshape(mame);
		author(EDGE_BRI);
		if (optz[o_stars]) showstars();
		sendframe();
	}

	// zoom and fade logo off screen
	bright = 30;
	for (count=0;count<60;count++)
	{
		mame.scale.x += 2;
		mame.scale.y = mame.scale.x;
		bright -= 0.5;
		mame.bright = bright;
		drawshape(mame);
		author(EDGE_BRI);
		if (optz[o_stars]) showstars();
		sendframe();
	}

	// set mame logo settings for screen saver
	mame.inc.x = ((NewXYInc() * NewDir() ) / 4) + 0.25;
	mame.inc.y = ((NewXYInc() * NewDir() ) / 4) + 0.25;
	mame.scale.x = 1.0;
	mame.scale.y = 1.0;
	mame.bright = EDGE_BRI;

	return mame;
}

/*************************************
	Define an asteroid vector object
**************************************/
vObject make_asteroid(void)
{
	vObject	asteroid;
	int ast_type = NewAst();
	//printf("Ast Type: %d\n", ast_type);
	static int ast1[] = {	2,0,5,1, 5,1,6,0, 6,0,8,2, 8,2,5,4, 5,4,8,5, 8,5,8,6,
							8,6,5,8, 5,8,2,8, 2,8,3,6, 3,6,0,6, 0,6,0,3, 0,3,2,0 };
	static int ast2[] = { 	2,0,3,1, 3,1,6,0, 6,0,8,3, 8,3,6,5, 6,5,8,6, 8,6,6,8,
							6,8,4,7, 4,7,2,8, 2,8,0,6, 0,6,1,4, 1,4,0,2, 0,2,2,0 };
	static int ast3[] = { 	2,0,4,3, 4,3,4,0, 4,0,6,0, 6,0,8,3, 8,3,8,5, 8,5,6,8,
							6,8,3,8, 3,8,0,5, 0,5,2,4, 2,4,0,3, 0,3,2,0 };
	static int ast4[] = { 	2,0,5,0, 5,0,8,2, 8,2,7,4, 7,4,8,6, 8,6,6,8, 6,8,4,6,
							4,6,2,8, 2,8,0,6, 0,6,0,2, 0,2,2,0 };

	switch (ast_type)
	{
	case 0:
		asteroid.outline.array = ast1;
		asteroid.outline.size = sizeof(ast1)/sizeof(*ast1);
		break;
	case 1:
		asteroid.outline.array = ast2;
		asteroid.outline.size = sizeof(ast2)/sizeof(*ast2);
		break;
	case 2:
		asteroid.outline.array = ast3;
		asteroid.outline.size = sizeof(ast3)/sizeof(*ast3);
		break;
	default:
		asteroid.outline.array = ast4;
		asteroid.outline.size = sizeof(ast4)/sizeof(*ast4);
		break;
	}
	asteroid.pos.x = NewXPos();
	asteroid.pos.y = NewYPos();
	asteroid.inc.x = ((NewXYInc() * NewDir() ) / 5) + 0.25;
	asteroid.inc.y = ((NewXYInc() * NewDir() ) / 5) + 0.25;
	ast_type = NewXYInc(); // 1- 10
	switch (ast_type)
	{
	case 1:
	case 2:
		asteroid.scale.x = 7;
		break;
	case 3:
	case 4:
	case 5:
		asteroid.scale.x = 4;
		break;
	default:
		asteroid.scale.x = 2;
		break;
	}
	asteroid.scale.y = asteroid.scale.x;
	asteroid.angle = 0;
	asteroid.theta = (NewTheta() * NewDir());
	asteroid.cent.x = 4;
	asteroid.cent.y = 4;
	if (colours[c_col][c_asts] == 99990) asteroid.colour = NewAstColour();
	else asteroid.colour = colours[c_col][c_asts]%7;
	//printf("Asteroid colour: %d\n", asteroid.colour);
	asteroid.bright = colours[c_int][c_asts];
	asteroid.edge	= 1;				// wrap

	return asteroid;
}


/**************************************
	 See if you really want to quit
**************************************/
int reallyescape(void)
{
	int count=0, timer=0, leave=0, cc=0;
	char timeout[33];
	printf("Press quit again to exit\n");
	while (timer < 1800)
	{
		for (count=0; count < NUM_ASTEROIDS; count++)
		{
			drawshape(asteroid[count]);
			asteroid[count] = updateobject(asteroid[count]);
		}
		if (optz[o_stars]) showstars();
		if ((timer % 120 == 0) || (timer % 120 == 60)) setLEDs(timer % 120 < 60 ? (N_LED | C_LED) : 0); // flash LEDs
		drawborders(200, 80, -200, -80, 1, 1, vyellow);
		itoa(30 -(timer/60), timeout, 10);
		setcolour(vblue, EDGE_BRI);
		PrintString("Press quit again to exit", 0, 40, 0, 5, 5, 0);
		PrintString("Any other key to return", 0, 0, 0, 5, 5, 0);
		setcolour(vcyan, EDGE_NRM);
		PrintString(timeout, 0, -40, 0, 5, 5, 0);
		sendframe();
		timer++;
		cc=getkey();
		if (cc)	  // a key was pressed
		{
			if (cc == keyz[k_quit]) leave=1;
			break;								// break out of loop
		}
	}
	setLEDs(0);
	return leave;
}

/*********************************
 Initialise the Sega Logo graphic
**********************************/
vObject make_sega(void)
{
	// 0 - 297, 0 - 74
	static int segalogo[] = {
	//S
	0,1,49,1,		49,1,62,14,		62,14,62,34,	62,34,45,50,	45,50,19,50,	19,50,19,52,	19,52,61,52,	61,52,61,74,
	61,74,13,74,	13,74,-1,60,	-1,60,-1,41,	-1,41,17,25,	17,25,39,25,	39,25,39,23,	39,23,0,23,		0,23,0,1,
	//s
	0,12,40,12,		40,12,49,20,	49,20,49,29,	49,29,41,37,	41,37,18,37,
	18,37,11,43,	11,43,11,56,	11,56,18,64,	18,64,61,63,
	//E
	133,74,87,74,	87,74,72,62,	72,62,72,15,	72,15,86,1,		86,1,133,1,		133,1,133,23,	133,23,101,23, 101,23,101,26,
	101,26,122,26, 122,26,122,50, 122,50,101,50, 101,50,101,53, 101,53,133,53, 133,53,133,74,
	//e
	133,64,95,64,	95,64,85,54,	85,54,85,21,	85,21,96,12,	96,12,133,12,	86,39,122,39,
	//G
	205,74,157,74, 157,74,143,61, 143,61,143,16, 143,16,161,1,	161,1,206,1,	206,1,206,47,	206,47,177,47, 177,47,177,25,
	177,25,181,25, 181,25,181,21, 181,21,173,21, 173,21,173,52, 173,52,205,52, 205,52,205,74,
	//g
	205,64,166,64, 166,64,157,57, 157,57,157,20, 157,20,167,11, 167,11,194,11, 194,11,194,35, 194,35,177,35,
	//A
	210,1,239,65,	239,65,248,74, 248,74,257,74, 257,74,266,65, 266,65,297,1,	297,1,250,1,
	250,1,250,26,	250,26,261,26, 261,26,254,39, 254,39,238,1,	238,1,210,1,
	//a
	223,0,251,59,	251,59,257,59, 257,59,278,14, 278,14,251,14 };

	vObject	sega;// 0 - 297, 0 - 74
	sega.outline.array = segalogo;
	sega.outline.size = sizeof(segalogo)/sizeof(*segalogo);
	sega.pos.x = -350;
	sega.pos.y = 0;
	sega.inc.x = 0;
	sega.inc.y = 0;
	sega.scale.x = 1;
	sega.scale.y = 1;
	sega.angle = 90;
	sega.theta = 0;
	sega.cent.x = 149;
	sega.cent.y = 37;
	sega.colour = vcyan;
	sega.bright = EDGE_BRI;
	sega.edge  = 1;			  // wrap

	return sega;
}


/******************************************
 Initialise the cinematronics Logo graphic
*******************************************/
vObject make_cinematronics(void)
{
	//x = 0 - 185, y= 0 - 93
	static int cinematronicslogo[] = {
	0,19,9,11,		9,11,24,0,		24,0,162,0,		162,0,178,11,	178,11,185,19, 185,19,156,47,
	156,47,124,47, 124,47,124,38, 124,38,63,38,	63,38,79,79,	79,79,106,79,	106,79,110,69,
	110,69,127,69, 127,69,127,77, 127,77,111,93, 111,93,74,93,	74,93,0,19,		9,11,178,11,
	//c
	11,17,176,17,	176,17,178,19, 178,19,153,45, 153,45,128,45, 128,45,135,34,
	135,34,51,34,	51,34,79,85,	79,85,108,85,	108,85,112,77, 112,77,124,77,
	124,77,110,91, 110,91,75,91,	75,91,9,19,		9,19,11,17 };

	vObject	cinematronics;
	cinematronics.outline.array = cinematronicslogo;
	cinematronics.outline.size = sizeof(cinematronicslogo)/sizeof(*cinematronicslogo);
	cinematronics.pos.x = -350;
	cinematronics.pos.y = 0;
	cinematronics.inc.x = 0;
	cinematronics.inc.y = 0;
	cinematronics.scale.x = 1;
	cinematronics.scale.y = 1;
	cinematronics.angle = 0;
	cinematronics.theta = 0;
	cinematronics.cent.x = 93;
	cinematronics.cent.y = 47;
	cinematronics.colour = vyellow;
	cinematronics.bright = EDGE_BRI;
	cinematronics.edge  = 1;			  // wrap

	return cinematronics;
}

/***********************************
 Initialise the centuri Logo graphic
************************************/
vObject make_centuri(void)
{
	//x = 0 - 114, y= 0 - 89
	static int centurilogo[] = {
	42,47,42,60,	42,60,34,68,	34,68,7,68,		7,68,0,60,		0,60,0,6,
	0,6,7,0,			7,0,35,0,		35,0,42,7,		42,7,42,18,		42,18,32,18,
	32,18,32,11,	32,11,14,11,	14,11,14,54,	14,54,31,54,	31,54,31,47,
	31,47,42,47,	96,54,96,68,	96,68,53,68,	53,68,53,0,		53,0,96,0,
	96,0,96,12,		96,12,68,12,	68,12,68,26,	68,26,82,26,	82,26,82,42,
	82,42,68,42,	68,42,68,54,	68,54,96,54,	146,68,132,68, 132,68,132,52,
	132,52,116,68, 116,68,103,68, 103,68,103,0,	103,0,117,0,	117,0,117,46,
	117,46,132,34, 132,34,132,0,	132,0,146,0,	146,0,146,68,	199,54,199,68,
	199,68,157,68, 157,68,157,54, 157,54,170,54, 170,54,170,0,	170,0,185,0,
	185,0,185,54,	185,54,199,54, 249,68,235,68, 235,68,235,12, 235,12,221,12,
	221,12,221,68, 221,68,207,68, 207,68,207,6,	207,6,214,0,	214,0,242,0,
	242,0,249,6,	249,6,249,68,	299,60,291,68, 291,68,257,68, 257,68,257,0,
	257,0,270,0,	270,0,270,24,	270,24,289,10, 289,10,289,0,	289,0,299,0,
	299,0,299,17,	299,17,290,25, 290,25,299,35, 299,35,299,60, 289,43,289,54,
	289,54,270,54, 270,54,270,43, 270,43,289,43, 336,54,336,68, 336,68,307,68,
	307,68,307,54, 307,54,314,54, 314,54,314,11, 314,11,307,11, 307,11,307,0,
	307,0,336,0,	336,0,336,11,	336,11,329,11, 329,11,329,54, 329,54,336,54 };

	vObject	centuri;
	centuri.outline.array = centurilogo;
	centuri.outline.size = sizeof(centurilogo)/sizeof(*centurilogo);
	centuri.pos.x = -350;
	centuri.pos.y = 0;
	centuri.inc.x = 0;
	centuri.inc.y = 0;
	centuri.scale.x = 1;
	centuri.scale.y = 1;
	centuri.angle = 0;
	centuri.theta = 0;
	centuri.cent.x = 173;
	centuri.cent.y = 34;
	centuri.colour = vwhite;
	centuri.bright = EDGE_BRI;
	centuri.edge  = 1;			  // wrap

	return centuri;
}

/**********************************
 Initialise the atari Logo graphic
***********************************/
vObject make_atari(void)
{
	//x = 0 - 114, y= 0 - 89
	static int atarilogo[] = {
	// Left
	0,0,10,1,		10,1,17,4,		17,4,29,13,		29,13,36,22,	36,22,43,34,
	43,34,46,49,	46,49,46,89,	46,89,39,89,	39,89,39,62,	39,62,35,46,
	35,46,29,34,	29,34,20,23,	20,23,8,15,		8,15,0,13,		0,13,0,0,
	// Middle
	51,0,51,89,		51,89,63,89,	63,89,63,0,		63,0,51,0,
	// Right
	114,0,104,1,	104, 1,97,4,	97,4,85,13,		85,13,78,22,	78,22,71,34,
	71,34,68,49,	68,49,68,89,	68,89,75,89,	75,89,75,62,	75,62,79,46,
	79,46,85,34,	85,34,94,23,	94,23,106,15,	106,15,114,13, 114,13,114,0 };

	vObject	atari;
	atari.outline.array = atarilogo;
	atari.outline.size = sizeof(atarilogo)/sizeof(*atarilogo);
	atari.pos.x = -350;
	atari.pos.y = 0;
	atari.inc.x = 0;
	atari.inc.y = 0;
	atari.scale.x = 1;
	atari.scale.y = 1;
	atari.angle = 0;
	atari.theta = 0;
	atari.cent.x = 57;
	atari.cent.y = 46;
	atari.colour = vwhite;
	atari.bright = EDGE_BRI;
	atari.edge	= 1;				// wrap

	return atari;
}


/***************************************
 Initialise the vectorbeam Logo graphic
****************************************/
vObject make_vbeam(void)
{
	//x = 0 - 496, y= 0 - 56
	static int vbeamlogo[] = {
	68,56,76,48,	76,48,68,40,	76,48,0,48,		0,48,0,4,		0,4,496,4,		496,4,496,48,	496,48,420,48,
	92,48,104,20, 104,20,116,48, 148,48,132,48,	132,48,132,20, 132,20,148,20, 148,36,132,36, 176,48,160,48,
	160,48,160,20, 160,20,176,20, 216,48,192,48, 204,48,204,20, 244,48,228,48,	228,48,228,20, 228,20,244,20,
	244,20,244,48, 276,20,260,36, 260,36,276,36, 276,36,276,48, 276,48,256,48, 256,48,256,20, 304,48,288,48,
	288,48,288,20, 288,20,304,20, 304,20,304,48, 304,36,292,36, 336,48,316,48, 316,48,316,20, 316,20,336,20,
	336,36,320,36, 376,20,364,48,	364,48,348,20, 356,36,368,36, 404,20,404,48, 404,48,396,36, 396,36,384,48,
	384,48,384,20 };

	vObject	vbeam;
	vbeam.outline.array = vbeamlogo;
	vbeam.outline.size = sizeof(vbeamlogo)/sizeof(*vbeamlogo);
	vbeam.pos.x = -350;
	vbeam.pos.y = 0;
	vbeam.inc.x = 0;
	vbeam.inc.y = 0;
	vbeam.scale.x = 1;
	vbeam.scale.y = 1;
	vbeam.angle = 0;
	vbeam.theta = 0;
	vbeam.cent.x = 248;
	vbeam.cent.y = 28;
	vbeam.colour = vyellow;
	vbeam.bright = EDGE_BRI;
	vbeam.edge	= 1;				// wrap

	return vbeam;
}

/***********************************
 Initialise the Midway Logo graphic
************************************/
vObject make_midway(void)
{
	//x = 0 - 496, y= 0 - 56
	static int midwaylogo[] = {
	//M outline from top left ccw
	84,184,32,36,		32,36,56,36,		56,36,68,72,		68,72,68,36,		68,36,84,36,
	84,36,108,72,		108,72,96,36,		96,36,116,36,		116,36,170,184,	170,184,140,184,
	140,184,108,140,	108,140,108,184, 	108,184,84,184,
	//Big outline from top left ccw
	68,216,0,4, 		0,4,128,4, 			128,4,200,216, 	200,216,68,216,
	//left divider
	34,108,56,108,
	//middle of M's
	80,108,120,108,	80,108,92,148,		92,148,92,108,		88,108,88,76,
	88,76,112,108,		108,108,136,148,	136,148,120,108,
	//right divider
	142,108,162,108 };

	vObject	midway;
	midway.outline.array = midwaylogo;
	midway.outline.size = sizeof(midwaylogo)/sizeof(*midwaylogo);
	midway.pos.x = -350;
	midway.pos.y = 0;
	midway.inc.x = 0;
	midway.inc.y = 0;
	midway.scale.x = 1;
	midway.scale.y = 1;
	midway.angle = 0;
	midway.theta = 0;
	midway.cent.x = 100;
	midway.cent.y = 108;
	midway.colour = vwhite;
	midway.bright = EDGE_BRI;
	midway.edge  = 1;				 // wrap

	return midway;
}


/***********************************************
 Print author information at the bottom of the screen
************************************************/
void author(int bright)
{
	if (bright > 25) bright = 25;
	setcolour(vwhite, bright);
	PrintString(auth1, xmax-170, -(ymax-45), 0, 4, 4, 0);
	setcolour(vwhite, (bright/2));
	PrintString(auth2, xmax-170, -(ymax-25), 0, 4, 4, 0);
}


/***********************************************
 Print press a key for menu message
************************************************/
void pressakey(int x, int y)
{
	if (x < (-xmax + 156))	x = -xmax + 156;
	if (x > (xmax - 156))	x = xmax - 156;
	if (y < (-ymax + 65))	y = -ymax + 65;	// stay off bottom few lines - don't clash with (c) text
	if (y > (ymax - 20))		y = ymax - 20;
	setcolour(vyellow, EDGE_NRM);
	PrintString("PRESS ANY CONTROL FOR MENU", x, y, 0, 4, 4, 0);
}


/***********************************************
 Set colour to one of red, green, blue, magenta,
 							 cyan, yellow, white
************************************************/
void setcolour(int clr, int bright)
{
	int r, g, b;
	if (clr > 7) clr = vwhite;
	if (bright > 31) bright = 31;
	GetRGBfromColour(clr, &r, &g, &b);
	if (ZVGPresent)
	{
		zvgFrameSetRGB15(r*bright, g*bright, b*bright);
	}
	SDL_VC = clr;		// very hacky!
	SDL_VB = bright;	// should pass as parameters to draw functions
}


/***********************************************
 Set colour to one of red, green, blue, magenta, cyan, yellow, white
************************************************/
void GetRGBfromColour(int clr, int *r, int *g, int *b)
{
	*r = ((clr == vred)   || (clr == vmagenta) || (clr == vyellow) || (clr == vwhite));
	*g = ((clr == vgreen) || (clr == vyellow)  || (clr == vcyan)   || (clr == vwhite));
	*b = ((clr == vblue)  || (clr == vmagenta) || (clr == vcyan)   || (clr == vwhite));
}


/***********************************************
 Print credits
************************************************/
int credits(void)
{
	int period = 60;		// timeframe for the animation to complete in frames
	int t = 0;
	int lines = 9;		  // lines of text
	int angle = 0;
	int bright = 0, colour = 1, top = 300;
	//float fall = 90;
	int x = X_MIN;
	int count = 0;
	int scale, start, finish, l;
	int breakout = 0;
	int bnw = (ZvgIO.envMonitor & MONF_BW) && ZVGPresent;
	char credits[lines][60];

	strcpy(credits[0], "Thanks go to the following");
	strcpy(credits[1], " ");
	strcpy(credits[2], "Zektor for the ZVG");
	strcpy(credits[3], "The MAME team");
	strcpy(credits[4], "Ian Boffin for the original menu and artwork");
	strcpy(credits[5], "Danny Pearson and Barry Shilmover for testing");
	strcpy(credits[6], "and an endless stream of suggestions");
	strcpy(credits[7], "Atari, Sega, Cinematronics et al for the games");
	strcpy(credits[8], "And all the vectorheads for keeping them alive");

	for (t=0; t<lines*(period/2);t++)
	{
		for (l=0;l<lines;l++)
		{
			start=(l*(period/4));
			finish=start+period;
			count=((t-start)%period);				// count = 0 to 59 - how far through anim time we are

			if ((t > start) && (t < finish))		// we are in animation time
			{
				//angle=fall-((fall/period)*count);
				angle=0;
				scale=((period+2)-count)*3;
				bright=count/4;
				//x=(X_MIN+((X_MAX/period)*count))*2;
				x=0;
			}
			else											// in display time
			{
				angle=0;
				scale=5;
				bright=18;
				x=0;
			}
			if (t>start)
			{
				//printf("t: %d Line: %d value: %d Colour: %s\n", t, l, (((t/2)%7)+l)%7, cols[(((t/2)%7)+l)%7]);
				if (bnw) colour = (l%7);
				else colour = ((((t+l)%31)+1)/4)%7;
				//printf("%d ", colour);
				setcolour( colour, bright);
				PrintString(credits[l], x, top - l*60, angle, scale, scale, angle);
			}
		}
		author(EDGE_BRI);
		if (optz[o_stars]) showstars();
//		if ((t%45) == 0)  setLEDs(0);
		if ((t%45) == 0) setLEDs(S_LED);
		if ((t%45) == 15) setLEDs(N_LED);
		if ((t%45) == 30) setLEDs(C_LED);

		if (breakout > 0)
		{
			break;
		}
		breakout += getkey();
		sendframe();
	}
	for (t=lines*(period/2);t>0;t--)
	{
		for (l=0;l<lines;l++)
		{
			start=(l*(period/4));
			finish=start+period;
			count=((t-start)%period);
			if ((t > start) && (t < finish))		// in animation time
			{
				//angle=((fall/period)*count)-fall;
				angle=0;
				scale=((period+3)-count)*3;
				bright=count/4;
				//x=(X_MAX-((X_MAX/period)*count))*2;
				x=0;
			}
			else											// in display time
			{
				angle=0;
				scale=5;
				bright=18;
				x=0;
			}
			if (t>start)
			{
				if (bnw) colour = (l%7);
				else colour = ((((t+l)%31)+1)/4)%7;
				// colour = ((((t+l)%31)+1)/4)%7;
				setcolour( colour, bright);
				PrintString(credits[l], x, top - l*60, angle, scale, scale, angle);
			}
		}
		author(EDGE_BRI);
		if (optz[o_stars]) showstars();
		if ((t%45) == 0) setLEDs(C_LED);
		if ((t%45) == 15) setLEDs(N_LED);
		if ((t%45) == 30) setLEDs(S_LED);

		if (breakout > 0)
		{
			break;
		}
		breakout += getkey();
		sendframe();
	}
	return breakout;
}

/*************************************
	Define a star vector object
**************************************/
vStar make_star(void)
{
	int	origin = NewAst();
	vStar  star;
	star.pos.x = NewXPos();
	star.pos.y = NewYPos();
	switch (origin)		// move star to random edge
	{
	case 0:
		star.pos.x=-xmax;
		break;
	case 1:
		star.pos.y=ymax;
		break;
	case 2:
		star.pos.x=xmax;
		break;
	default:
		star.pos.y=-ymax;
		break;
	}
	//star.delta.x = (0 - star.pos.x)/100;
	//star.delta.y = (0 - star.pos.y)/100;
	star.delta.x = (star.pos.x/xmax)/15;
	star.delta.y = (star.pos.y/ymax)/15;
	star.change.x = star.delta.x;
	star.change.y = star.delta.y;
	star.pos.x = star.delta.x * 500;
	star.pos.y = star.delta.y * 500;
	star.speed = NewStarSpeed();
	//printf("X: %f Y: %f Dx: %f Dy: %f Speed: %f\n", star.pos.x, star.pos.y, star.delta.x, star.delta.y, star.speed);
	return star;
}

/********************************************************************
Update a star object:
	* increment x and y positions
	* generate new star at centre if edge hit
********************************************************************/
vStar updatestar(vStar star)
{
	star.pos.x = star.pos.x + (star.change.x * star.speed);
	star.pos.y = star.pos.y + (star.change.y * star.speed);
	star.change.x = star.change.x + star.delta.x;
	star.change.y = star.change.y + star.delta.y;

	if (star.pos.x > xmax || star.pos.x < -xmax || star.pos.y > ymax || star.pos.y < -ymax)
	{
		star = make_star();
	}
	return star;
}

/*******************************************************************
 Draw a star
********************************************************************/
void drawstar(vStar star)
{
	point last;
	if (star.change.x != 0 && star.change.y != 0)
	{
		last.x = star.pos.x - star.change.x;
		last.y = star.pos.y - star.change.y;
		setcolour(vwhite, sqrt((star.pos.x*star.pos.x)+(star.pos.y*star.pos.y))/25);
		drawvector(star.pos, last, 0, 0);
	}
}

/*******************************************************************
 Display the stars on screen
********************************************************************/
void showstars()
{
	int c;
	for (c=0; c < (NUM_STARS); c++)
	{
		starz[c] = updatestar(starz[c]);
		drawstar(starz[c]);
	}
}


/******************************************************************
Read the cfg file settings
*******************************************************************/
void getsettings(void)
{
	int c = 0;
	// interface settings
	optz[o_rot] 		= iniparser_getint(ini, "interface:rotation", 0);
	optz[o_rot] 		= abs(optz[o_rot]%4);
	optz[o_stars]		= iniparser_getboolean(ini, "interface:stars", 0);
	optz[o_ucase]		= iniparser_getboolean(ini, "interface:caps", 0);
	optz[o_togpnm]		= iniparser_getboolean(ini, "interface:showpnm", 0);
	optz[o_smenu]		= iniparser_getboolean(ini, "interface:smartmenu", 1);
	optz[o_redozvg]		= iniparser_getboolean(ini, "interface:reopenzvg", 1);
	optz[o_dovga]		= iniparser_getboolean(ini, "interface:rendervga", 0);
	optz[o_attmode]		= iniparser_getboolean(ini, "interface:attractmode", 0);
	
	strcpy(attractargs, iniparser_getstring(ini, "interface:attractargs", "-attract -str 30"));
	strcpy(zvgargs, iniparser_getstring(ini, "interface:zvgargs", "-video zvg"));

	// controllers
	optz[o_mouse]		= iniparser_getint(ini, "controls:spinnertype", 0);
	optz[o_mouse]		= abs(optz[o_mouse]%3);
	optz[o_mswapXY]		= iniparser_getboolean(ini, "controls:swapaxes", 0);
	optz[o_msens] 		= iniparser_getint(ini, "controls:spinsens", 30);
	optz[o_msens] 		= abs(optz[o_msens]%100);
	optz[o_msamp] 		= iniparser_getint(ini, "controls:spinsamp", 6);
	optz[o_msamp] 		= abs(optz[o_msamp]%20);
	optz[o_mrevX] 		= iniparser_getboolean(ini, "controls:reversexaxis", 0);
	optz[o_mrevY] 		= iniparser_getboolean(ini, "controls:reverseyaxis", 0);
	optz[o_mpoint]		= iniparser_getboolean(ini, "controls:pointer", 0);
	if (optz[o_msens] < 1) optz[o_msens] = 1;
	if (optz[o_msamp] < 1) optz[o_msamp] = 1;

	// key bindings - global keys
	keyz[k_menu]		= iniparser_getint(ini, "keys:k_togglemenu", HYPSPACE);
	keyz[k_quit]		= iniparser_getint(ini, "keys:k_quit", ESC);
	keyz[k_random]		= iniparser_getint(ini, "keys:k_random", 127);
	keyz[k_options]		= iniparser_getint(ini, "keys:k_options", TILDE);

	// Manufacturer menu keys
	keyz[k_pman]		= iniparser_getint(ini, "keys:k_prevman", LEFT);
	keyz[k_nman]		= iniparser_getint(ini, "keys:k_nextman", RIGHT);

	// Game list menu keys
	keyz[k_pgame]		= iniparser_getint(ini, "keys:k_prevgame", LEFT);
	keyz[k_ngame]		= iniparser_getint(ini, "keys:k_nextgame", RIGHT);
	keyz[k_pclone] 		= iniparser_getint(ini, "keys:k_prevclone", THRUST);
	keyz[k_nclone] 		= iniparser_getint(ini, "keys:k_nextclone", FIRE);
	keyz[k_start]		= iniparser_getint(ini, "keys:k_startgame", START1);

	// now the colour and brightness values
	colours[c_col][c_glist]	= getcolour(iniparser_getstring(ini, "colours:c_gamelist", "green"));
	colours[c_int][c_glist]	= iniparser_getint(ini, "colours:i_gamelist", EDGE_NRM);
	colours[c_col][c_sgame]	= getcolour(iniparser_getstring(ini, "colours:c_selgame", "white"));
	colours[c_int][c_sgame]	= iniparser_getint(ini, "colours:i_selgame", EDGE_BRI);
	colours[c_col][c_sman]	= getcolour(iniparser_getstring(ini, "colours:c_selman", "red"));
	colours[c_int][c_sman]	= iniparser_getint(ini, "colours:i_selman", EDGE_BRI);
	colours[c_col][c_man]	= getcolour(iniparser_getstring(ini, "colours:c_man", "red"));
	colours[c_int][c_man]	= iniparser_getint(ini, "colours:i_man", EDGE_NRM);
	colours[c_col][c_pnman]	= getcolour(iniparser_getstring(ini, "colours:c_pnman", "red"));
	colours[c_int][c_pnman]	= iniparser_getint(ini, "colours:i_pnman", EDGE_NRM);
	colours[c_col][c_arrow]	= getcolour(iniparser_getstring(ini, "colours:c_arrow", "yellow"));
	colours[c_int][c_arrow]	= iniparser_getint(ini, "colours:i_arrow", EDGE_NRM);
	colours[c_col][c_asts]	= getcolour(iniparser_getstring(ini, "colours:c_asteroids", "white"));
	colours[c_int][c_asts]	= iniparser_getint(ini, "colours:i_asteroids", EDGE_NRM);
	for (c=0;c<7;c++)
	{
		colours[c_int][c] %=32;
	}
	if ((keyz[k_pman] == keyz[k_pgame]) || (keyz[k_nman] == keyz[k_ngame])) optz[o_smenu] = 0;
}


/******************************************************************
Write a value to the cfg file
valtype: 0 = write value as-is
			1 = as hex
			2 = as a colour
			3 = as boolean yes/no
*******************************************************************/
void writeinival(char *key, int value, int force, int valtype)
{
	char buffer[10];
	char cols[7][10] = {"red", "magenta", "cyan", "blue", "yellow", "green", "white"};
	if ((iniparser_find_entry(ini, key)) || (force))
	{
		switch (valtype)
		{
		case 1:
			if (value > 0)
			{
				sprintf(buffer, "0x%04x", value);
				iniparser_set(ini, key, buffer);
			}
			break;
		case 2:
			if (value != 99990) iniparser_set(ini, key, cols[abs(value%7)]);
			else iniparser_set(ini, key, itoa(value, buffer, 10));
			break;
		case 3:
			iniparser_set(ini, key, ((value == 0) ? "no" : "yes"));
			break;
		default:
			iniparser_set(ini, key, itoa(value, buffer, 10));
			break;
		}
	}
}

/******************************************************************
Write the cfg file to the loaded dictionary
*******************************************************************/
void writecfg()
{
	// Ensure the [sections] exist
	if (!iniparser_find_entry(ini, "interface")) iniparser_set(ini, "interface", NULL);
	if (!iniparser_find_entry(ini, "controls")) iniparser_set(ini, "controls", NULL);
	if (!iniparser_find_entry(ini, "keys")) iniparser_set(ini, "keys", NULL);
	if (!iniparser_find_entry(ini, "colours")) iniparser_set(ini, "colours", NULL);

	// write the interface settings
	writeinival("interface:rotation",		optz[o_rot], 1, 0);
	writeinival("interface:stars",			optz[o_stars], 1, 3);
	writeinival("interface:caps",			optz[o_ucase], 1, 3);
	writeinival("interface:showpnm",		optz[o_togpnm], 1, 3);
	writeinival("interface:smartmenu",		optz[o_smenu], 1, 3);
	writeinival("interface:reopenzvg",		optz[o_redozvg], 1, 3);
	writeinival("interface:rendervga", 		optz[o_dovga], 0, 3);
	writeinival("interface:attractmode",	optz[o_attmode], 0, 3);
	iniparser_set(ini, "interface:attractargs", attractargs);
	iniparser_set(ini, "interface:zvgargs", zvgargs);

	// write the spinner/mouse settings
	writeinival("controls:spinnertype",		optz[o_mouse], 1, 0);
	writeinival("controls:swapaxes",		optz[o_mswapXY], optz[o_mouse], 3);
	writeinival("controls:spinsens",		optz[o_msens], optz[o_mouse], 0);
	writeinival("controls:spinsamp",		optz[o_msamp], optz[o_mouse], 0);
	writeinival("controls:reversexaxis",	optz[o_mrevX], optz[o_mouse], 3);
	writeinival("controls:reverseyaxis",	optz[o_mrevY], optz[o_mouse], 3);
	writeinival("controls:pointer",			optz[o_mpoint], 0, 3);

	// write the key bindings
	writeinival("keys:k_togglemenu",		keyz[k_menu], 1, 1);
	writeinival("keys:k_quit",				keyz[k_quit], 1, 1);
	writeinival("keys:k_options",			keyz[k_options], 1, 1);
	writeinival("keys:k_random",			keyz[k_random], 0, 1);
	writeinival("keys:k_prevman",			keyz[k_pman], 1, 1);
	writeinival("keys:k_nextman",			keyz[k_nman], 1, 1);
	writeinival("keys:k_prevgame",			keyz[k_pgame], 1, 1);
	writeinival("keys:k_nextgame",			keyz[k_ngame], 1, 1);
	writeinival("keys:k_prevclone",			keyz[k_pclone], 1, 1);
	writeinival("keys:k_nextclone",			keyz[k_nclone], 1, 1);
	writeinival("keys:k_startgame",			keyz[k_start], 1, 1);

	// write the colour settings
	writeinival("colours:c_gamelist",		colours[c_col][c_glist], 1, 2);
	writeinival("colours:i_gamelist",		colours[c_int][c_glist], 1, 0);
	writeinival("colours:c_selgame",		colours[c_col][c_sgame], 1, 2);
	writeinival("colours:i_selgame",		colours[c_int][c_sgame], 1, 0);
	writeinival("colours:c_selman",			colours[c_col][c_sman], 1, 2);
	writeinival("colours:i_selman",			colours[c_int][c_sman], 1, 0);
	writeinival("colours:c_man",			colours[c_col][c_man], 1, 2);
	writeinival("colours:i_man",			colours[c_int][c_man], 1, 0);
	writeinival("colours:c_pnman",			colours[c_col][c_pnman], 1, 2);
	writeinival("colours:i_pnman",			colours[c_int][c_pnman], 1, 0);
	writeinival("colours:c_arrow",			colours[c_col][c_arrow], 1, 2);
	writeinival("colours:i_arrow",			colours[c_int][c_arrow], 1, 0);
	writeinival("colours:c_asteroids",		colours[c_col][c_asts], 0, 2);
	writeinival("colours:i_asteroids",		colours[c_int][c_asts], 0, 0);
}


/******************************************************************
Get a colour from the cfg file and convert to an integer
*******************************************************************/
int getcolour(char *colval)
{
	char cols[8][10] = {"RED", "MAGENTA", "CYAN", "BLUE", "YELLOW", "GREEN", "WHITE", "99990"};
	int c, value;
	char temp[20];
	strcpy(temp, colval);
	ucase(temp);
	value = vwhite; //default to white
	for (c=0; c<8; c++)
	{
		if (!strcmp(temp, cols[c])) value = c;
	}
	if (value == 7) value = 99990;
	return value;
}


/********************************************************************
	Select a random game
********************************************************************/
g_node* GetRandomGame(m_node *gameslist)
{
	int	x=0, rf;
	m_node	*randommanuf;
	g_node	*randomgame, *randomclone;
	randommanuf = gameslist;
	randomgame	= randommanuf->firstgame;
	randomclone = randomgame;
	rf = (rand()/(RAND_MAX/totalnumgames+1));
	//printf("Random factor = %d\n", rf);

	while (x < rf)
	{
		if (randomclone->nclone != NULL)							// if not at last clone, next clone
		{
			randomclone = randomclone->nclone;
		}
		else
		{
			if (randomgame->next != randommanuf->firstgame) // if at bottom of list, next manuf
			{
				randomgame = randomgame->next;
				randomclone = randomgame;
			}
			else
			{
				randommanuf = randommanuf->nmanuf;
				randomgame = randommanuf->firstgame;
				randomclone = randomgame;
			}
		}
		x++;
	}
	return randomclone;
}


/********************************************************************
	Attract mode a random game
********************************************************************/
void PlayAttractGame(m_node *gameslist)
{
	g_node	*selectedgame;
	selectedgame = GetRandomGame(gameslist);
	size_t lf = strlen(attractargs);
	size_t ls = strlen(selectedgame->clone);
	char *args = (char*) malloc((lf + ls + 2) * sizeof(char));

	strcpy(args, attractargs);
	args[lf] = ' ';
	strcpy(&args[lf+1], selectedgame->clone);
	
	RunGame(args);
}


/******************************************************************
	Print a vector mouse pointer on screen at current mouse position
	Just for kicks - doesn't do anything but has potential...
*******************************************************************/
void PrintPointer(int mx, int my)
{
	//printf("mousex: %d mousey: %d\n", mx, my);
	setcolour(6, 25); // white, EDGE_BRI
	PrintString(">", mx, my, 135, 8, 4, 0);
}

/******************************************************************
	Interface options
*******************************************************************/
void SetOptions(void)
{
	int cc = 0, top = 250, cursor = 0, options;
	int optx = 0, opty = 0, timer = 0;
	point p1, p2;
	char angle[10];
	while ((cc != keyz[k_options]) && (cc != keyz[k_quit]) && timer < 1800)
	{
		timer++;
		if (timer%optz[o_msamp] == 0)
		{
			mousemick();			 // update mousexmick and mouseymick
		}
		else
		{
			mousexmick = 0;
			mouseymick = 0;
		}
		
		if (mousexmick < -optz[o_msens])
		{
			optx = optx - 30;
			if (optx < -xmax) optx = xmax;
			if (optz[o_mouse] == 1)
			{
				opty = opty - 30;
				if (opty < -ymax) opty = ymax;
			}
		}

		if (mousexmick > optz[o_msens])
		{
			optx = optx + 30;
			if (optx > xmax) optx = -xmax;
			if (optz[o_mouse] == 1)
			{
				opty = opty + 30;
				if (opty > ymax) opty = -ymax;
			}
		}
		if (mouseymick > optz[o_msens])
		{
			opty = opty - 30;
			if (opty < -ymax) opty = ymax;
		}
		if (mouseymick < -optz[o_msens])
		{
			opty = opty + 30;
			if (opty > ymax) opty = -ymax;
		}
		setcolour(vcyan, 15);
		p1.x = -xmax+24;
		p1.y = 0;
		p2.x = xmax-24;
		p2.y = 0;
		drawvector(p1, p2, 0, -ymax+24);
		drawvector(p1, p2, 0, ymax-24);
		p1.x = 0;
		p1.y = -ymax+24;
		p2.x = 0;
		p2.y = ymax-24;
		drawvector(p1, p2, -xmax+24, 0);
		drawvector(p1, p2, xmax-24, 0);
		p1.x = -xmax;
		p1.y = 0;
		p2.x = xmax;
		p2.y = 0;
		drawvector(p1, p2, 0, -ymax);
		drawvector(p1, p2, 0, ymax);
		p1.x = 0;
		p1.y = -ymax;
		p2.x = 0;
		p2.y = ymax;
		drawvector(p1, p2, -xmax, 0);
		drawvector(p1, p2, xmax, 0);

		if (optz[o_stars]) showstars();

		setcolour(vwhite, 15);
		if (cursor == 0)	setcolour(vwhite, 25);
		PrintString("Rotation       ", -150, top, 0, 6, 6, 0);
		if (optz[o_rot] == 0) strcpy(angle,"0      ");
		if (optz[o_rot] == 1) strcpy(angle,"90     ");
		if (optz[o_rot] == 2) strcpy(angle,"180    ");
		if (optz[o_rot] == 3) strcpy(angle,"270    ");
		PrintString(angle, 250, top, 0, 6, 6, 0);
		
		setcolour(vwhite, 15);
		if (cursor == 1)	setcolour(vwhite, 25);
		PrintString("Stars          ", -150, top - 50, 0, 6, 6, 0);
		PrintString(optz[o_stars] == 1 ? "yes    " : "no     ", 250, top - 50, 0, 6, 6, 0);
		
		setcolour(vwhite, 15);
		if (cursor == 2)	setcolour(vwhite, 25);
		PrintString("All Caps       ", -150, top - 100, 0, 6, 6, 0);
		PrintString((optz[o_ucase] == 1 ? "yes    " : "no     "), 250, top - 100, 0, 6, 6, 0);

		setcolour(vwhite, 15);
		if (cursor == 3)	setcolour(vwhite, 25);
		PrintString("Show Prev/next ", -150, top - 150, 0, 6, 6, 0);
		PrintString((optz[o_togpnm] == 1 ? "yes    " : "no     "), 250, top - 150, 0, 6, 6, 0);

		setcolour(vwhite, 15);
		if (cursor == 4)	setcolour(vwhite, 25);
		PrintString("Reopen ZVG     ", -150, top - 200, 0, 6, 6, 0);
		PrintString((optz[o_redozvg] == 1 ? "yes    " : "no     "), 250, top - 200, 0, 6, 6, 0);

		if (mousefound)
		{
			setcolour(vwhite, 15);
			if (cursor == 5)	setcolour(vwhite, 25);
			PrintString("Optical Control", -150, top - 250, 0, 6, 6, 0);
			if (optz[o_mouse] == 0) strcpy(angle,"None   ");
			if (optz[o_mouse] == 1) strcpy(angle,"Spinner");
			if (optz[o_mouse] == 2) strcpy(angle,"Mouse  ");
			PrintString(angle, 250, top - 250, 0, 6, 6, 0);

			if (optz[o_mouse])
			{
				options = 11;
				setcolour(vwhite, 15);
				p1.x = -275;
				p1.y = top - 280;
				p2.x = p1.x;
				p2.y = top - 500;
				drawvector(p1, p2, 0, 0);
				if (cursor == 6)	setcolour(vwhite, 25);
				PrintString("- Swap X/Y Axes ", -133, top - 300, 0, 6, 6, 0);
				PrintString((optz[o_mswapXY] == 1 ? "yes    " : "no     "), 250, top - 300, 0, 6, 6, 0);

				setcolour(vwhite, 15);
				if (cursor == 7)	setcolour(vwhite, 25);
				PrintString("- Reverse X Axis", -133, top - 350, 0, 6, 6, 0);
				PrintString((optz[o_mrevX] == 1 ? "yes    " : "no     "), 250, top - 350, 0, 6, 6, 0);
		
				setcolour(vwhite, 15);
				if (cursor == 8)	setcolour(vwhite, 25);
				PrintString("- Reverse Y Axis", -133, top - 400, 0, 6, 6, 0);
				PrintString((optz[o_mrevY] == 1 ? "yes    " : "no     "), 250, top - 400, 0, 6, 6, 0);

				setcolour(vwhite, 15);
				if (cursor == 9)	setcolour(vwhite, 25);
				PrintString("- Sample Rate   ", -133, top - 450, 0, 6, 6, 0);
				itoa(optz[o_msamp], angle, 10);
				PrintString(angle, 199 + (optz[o_msamp] > 9 ? 6 : 0), top - 450, 0, 6, 6, 0);

				setcolour(vwhite, 15);
				if (cursor == 10)	setcolour(vwhite, 25);
				PrintString("- Sensitivity   ", -133, top - 500, 0, 6, 6, 0);
				itoa(optz[o_msens], angle, 10);
				PrintString(angle, 199 + (optz[o_msens] > 9 ? 6 : 0), top - 500, 0, 6, 6, 0);

				setcolour(vyellow, 25);
				PrintString("X", optx, ymax-12, 0, 10, 5, 0);
				PrintString("X", optx, -ymax+12, 0, 10, 5, 0);
				PrintString("X", xmax-12, opty, 0, 10, 5, 0);
				PrintString("X", -xmax+12, opty, 0, 10, 5, 0);
			}
			else options = 6;
		}
		else options = 5;

		
		cc=getkey();
		if (cc) timer = 0;
		if (cc == keyz[k_ngame]) cursor = (cursor + 1) % options;
		if (cc == keyz[k_pgame])
		{
			cursor--;
			if (cursor < 0) cursor = (options - 1);
		}

		switch (cursor)
		{
			case 0:													// rotate screen through 90 degrees
			{
				if (cc == keyz[k_nclone]) optz[o_rot] = ((optz[o_rot]+1)%4);
				if (cc == keyz[k_pclone])
				{
					optz[o_rot]--;
					if (optz[o_rot] < 0) optz[o_rot] = 3;
				}
				if (cc == keyz[k_start]) optz[o_rot] = 0;
				if ((optz[o_rot] == 1) || (optz[o_rot] == 3))
				{
					xmax = Y_MAX;
					ymax = X_MAX;
				}
				else
				{
					xmax = X_MAX;
					ymax = Y_MAX;
				}
				break;
			}
			case 1:	// Stars
			{
				if (cc == keyz[k_pclone] || cc == keyz[k_nclone]) optz[o_stars] = !optz[o_stars];
				if (cc == keyz[k_start]) optz[o_stars]	= 0;
				break;
			}

			case 2:	// Upper/Lower case
			{
				if (cc == keyz[k_pclone] || cc == keyz[k_nclone]) optz[o_ucase] = !optz[o_ucase];
				if (cc == keyz[k_start]) optz[o_ucase] = 0;
				break;
			}
			
			case 3:	// Prev/Next manufacturer names
			{
				if (cc == keyz[k_pclone] || cc == keyz[k_nclone]) optz[o_togpnm] = !optz[o_togpnm];
				if (cc == keyz[k_start]) optz[o_togpnm] = 0;
				break;
			}
			
			case 4:	// re-open ZVG
			{
				if (cc == keyz[k_pclone] || cc == keyz[k_nclone]) optz[o_redozvg] = !optz[o_redozvg];
				if (cc == keyz[k_start]) optz[o_redozvg] = 0;
				break;
			}
			
			case 5:	// Optical controller
			{
				if (cc == keyz[k_nclone]) optz[o_mouse] = ((optz[o_mouse]+1)%3);
				if (cc == keyz[k_pclone])
				{
					optz[o_mouse]--;
					if (optz[o_mouse] < 0) optz[o_mouse] = 2;
				}
				if (cc == keyz[k_start]) optz[o_mouse] = 0;
				break;
			}

			case 6:	// swap mouse axes
			{
				if (cc == keyz[k_pclone] || cc == keyz[k_nclone]) optz[o_mswapXY] = !optz[o_mswapXY];
				if (cc == keyz[k_start]) optz[o_mswapXY] = 0;
				break;
			}
			
			case 7:	// reverse X axis
			{
				if (cc == keyz[k_pclone] || cc == keyz[k_nclone]) optz[o_mrevX] = !optz[o_mrevX];
				if (cc == keyz[k_start]) optz[o_mrevX] = 0;
				break;
			}
			
			case 8:	// reverse Y axis
			{
				if (cc == keyz[k_pclone] || cc == keyz[k_nclone]) optz[o_mrevY] = !optz[o_mrevY];
				if (cc == keyz[k_start]) optz[o_mrevY] = 0;
				break;
			}

			case 9:	// Mouse sample rate
			{			
				if (cc == keyz[k_pclone])
				{
					optz[o_msamp]--;											// decrease sample rate
					if (optz[o_msamp] < 1) optz[o_msamp] = 1;
				}
				if (cc == keyz[k_nclone])
				{
					optz[o_msamp]++;											// increase sample rate
					if (optz[o_msamp] > 20) optz[o_msamp] = 20;
				}
				if (cc == keyz[k_start]) optz[o_msamp] = 6;
				break;
			}

			case 10:	// Mouse sensitivity
			{
				if (cc == keyz[k_pclone])
				{
					optz[o_msens]--;											// decrease sensitivity
					if (optz[o_msens] < 0) optz[o_msens] = 0;
				}
				if (cc == keyz[k_nclone])
				{
					optz[o_msens]++;											// increase sensitivity
					if (optz[o_msens] > 100) optz[o_msens] = 100;
				}
				if (cc == keyz[k_start]) optz[o_msens] = 30;
			}
		}
		sendframe();
	}
}
