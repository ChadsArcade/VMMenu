// Linux specific functions for the Vector Mame Menu

#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <sys/io.h>
#include "SDL.h"
#include "SDL_gfxPrimitives.h"
#include "vmmstddef.h"
#include "LinuxVMM.h"
#include "zvgFrame.h"
#include "zvgPort.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

extern void	GetRGBfromColour(int, int*, int*, int*);					// Get R, G and B components of a passed colour

SDL_Surface*	screen;
int 				WINDOW_WIDTH = 512;
int 				WINDOW_HEIGHT = 384;
int 				WINDOW_SCALE = 2;
const 			char* WINDOW_TITLE = "VectorMameMenu";
int 				mdx=0, mdy=0;
int				mousexmick=0, mouseymick=0;
extern			int ZVGPresent, SDL_VB, SDL_VC;
char				zvgargs[30];
int 				optz[15];
int				LEDstate=0;


/******************************************************************
	Start up the ZVG if poss and use SDL if necessary
*******************************************************************/
void startZVG(void)
{
	uint error;
	InitialiseSDL(1);
	error = zvgFrameOpen();			// initialize ZVG
	if (error)
	{	zvgError( error);				// print error
		printf("ZVG not found, all rendering will be done to window.\n");
		ZVGPresent = 0;
	}
}

/******************************************************************
	itoa function, which isn't ANSI though is part of DJGPP
*******************************************************************/
char* itoa(int value, char* result, int base)
{
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}


/********************************************************************
 Initialise SDL and screen
********************************************************************/
void InitialiseSDL(int start)
{
	/* Initialise SDL */
	if (start)
	{
		if( SDL_Init( SDL_INIT_VIDEO ) < 0)
		{
			fprintf( stderr, "Could not initialise SDL: %s\n", SDL_GetError() );
			exit( -1 );
		}
		//else printf("Init: %s\n", SDL_GetError());
	}
//	const SDL_VideoInfo* ptrVidInfo = SDL_GetVideoInfo();
//	WINDOW_WIDTH = ptrVidInfo->current_w;
//	WINDOW_HEIGHT = ptrVidInfo->current_h;
//	WINDOW_SCALE = 1;
//	screen = SDL_SetVideoMode( WINDOW_WIDTH, WINDOW_HEIGHT, 0, SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);

	/* Set a video mode */
	screen = SDL_SetVideoMode( WINDOW_WIDTH, WINDOW_HEIGHT, 0, SDL_SWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption( WINDOW_TITLE, 0 );

	SDL_Event event;
	while (SDL_PollEvent(&event)) {}				// clear event buffer
	SDL_WM_GrabInput(SDL_GRAB_ON);
	while (SDL_PollEvent(&event)) {}				// clear event buffer
	SDL_ShowCursor(SDL_DISABLE);
	while (SDL_PollEvent(&event)) {}				// clear event buffer

//	printf("SDL opened...\n");
	// Move mouse to centre of window and discard the mouse deltas
	//SDL_WarpMouse(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
	//SDL_Event event;
	//while (SDL_PollEvent(&event)) {}
	//SDL_GetRelativeMouseState(&mousexmick, &mouseymick);
	//SDL_GetRelativeMouseState (NULL, NULL);	// reset mouse deltas
	while (SDL_PollEvent(&event)) {}				// clear event buffer
	mousexmick = 0;
	mouseymick = 0;
}


/********************************************************************
	Close off SDL cleanly
********************************************************************/
void CloseSDL(int done)
{
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	SDL_ShowCursor(SDL_ENABLE);
	SDL_FreeSurface(screen);
	if (done) SDL_Quit();
}

/********************************************************************
	Send a vector to the SDL surface - adjust co-ords from ZVG format
********************************************************************/
void SDLvector(float x1, float y1, float x2, float y2, int clr, int bright)
{
	int r, g, b;
	if (optz[o_dovga] || !ZVGPresent)
	{
		if (clr > 7) clr = vwhite;
		if (bright > 31) bright = 31;
		GetRGBfromColour(clr, &r, &g, &b);
		lineRGBA(screen, x1/WINDOW_SCALE+(WINDOW_WIDTH/2), -y1/WINDOW_SCALE+(WINDOW_HEIGHT/2),
							  x2/WINDOW_SCALE+(WINDOW_WIDTH/2),	-y2/WINDOW_SCALE+(WINDOW_HEIGHT/2),
							  r*8*bright, g*8*bright, b*8*bright, 255);
	}
}


/********************************************************************
	Send a screen to the SDL surface
********************************************************************/
void FrameSendSDL()
{
	if (optz[o_dovga] || !ZVGPresent)
	{
		if (!ZVGPresent) SDL_Delay(100/6);	// 1/60 of a sec, same as frame rate, in msecs
		SDL_Flip(screen);							// bring buffer to screen
		SDL_FillRect(screen, NULL, 0);		// clear the back buffer
	}
}

/******************************************************************
Check whether a mouse driver is installed
*******************************************************************/
int initmouse(void)
{
	return 1;
}

/******************************************************************
Get the amount by which the mouse has been moved since last check
	deltas are read and accumulated by keyboard event function
*******************************************************************/
void mousemick(void)
{
	int tempaxis;
//	SDL_PumpEvents();
//	printf("MickX: %d  MickY: %d\n",mousexmick, mouseymick);
//	SDL_GetRelativeMouseState(&mousexmick, &mouseymick);
//	printf("MickX: %d  MickY: %d\n",mousexmick, mouseymick);
	mousexmick = mdx;
	mouseymick = mdy;
	mdx = 0;
	mdy = 0;
	if (mousexmick | mouseymick)
	{
		if (optz[o_mswapXY]) // swap x and y axes
		{
			tempaxis = mousexmick;
			mousexmick = mouseymick;
			mouseymick = tempaxis;
		}
		if (optz[o_mrevX]) mousexmick = -mousexmick;
		if (optz[o_mrevY]) mouseymick = -mouseymick;
		if (optz[o_mouse] == 1) mouseymick = 0;
		//printf("MickX: %d  MickY: %d\n",mousexmick, mouseymick);
	}
}

/******************************************************************
Get keypress - SDL implementation. Returns scancode of pressed key
*******************************************************************/
int getkey(void)
{
	int key=0;

	SDL_Event event;
//	SDL_PollEvent( &event );					// Poll for events
	while(SDL_PollEvent(&event))
	{
		//printf("Event: %d\n", event.type);
		switch(event.type)
		{
			case SDL_MOUSEMOTION:
				mdx += event.motion.xrel;
				mdy += event.motion.yrel;
				break;
			case SDL_KEYDOWN:
				key = event.key.keysym.scancode;
//				return key;
				break;
			default:
				break;
		}
	}
	return key;
}


void mousepos(int *mx, int *my)
{
	SDL_GetMouseState(mx, my);
	*mx = (*mx - WINDOW_WIDTH/2) * 2;
	*my = (*my - WINDOW_HEIGHT/2) * -2;
}


/******************************************************************
	Write to keyboard LEDs value held in global variable LEDstate
 - only if state has changed
*******************************************************************/
void setLEDs(int leds)
{
	if (LEDstate != leds)
	{
		Display *dpy = XOpenDisplay(0);
		XKeyboardControl values;
		values.led_mode = leds & S_LED ? LedModeOn : LedModeOff;
		values.led = 3;
		XChangeKeyboardControl(dpy, KBLedMode, &values);
		XkbLockModifiers(dpy, XkbUseCoreKbd, C_LED | N_LED, leds & (C_LED | N_LED) );
		XFlush(dpy);
		XCloseDisplay(dpy);
		LEDstate = leds;
	}
}

/******************************************************
Send the frame to the ZVG, exit if it went pear shaped
*******************************************************/
int sendframe(void)
{
	uint	err=0;
	if (ZVGPresent)
	{
		tmrWaitForFrame();		// wait for next frame time
		err = zvgFrameSend();	// send next frame
		if (err)
		{
			zvgError( err);
			zvgFrameClose();		// fix up all the ZVG stuff
			exit(1);
		}
	}
	FrameSendSDL();
	return err;
}

/******************************************************
Close everything off for a graceful exit
*******************************************************/
void ShutdownAll(void)
{
	CloseSDL(1);
	setLEDs(0);											// restore LED status
	if (ZVGPresent)
	{
		zvgFrameClose();									// fix up all the ZVG stuff
	}
}

/*******************************************************************
 Draw a vector - pass the start, end points, and the x and y offsets
 Uses global rotation variable to determine orientation
********************************************************************/
void drawvector(point p1, point p2, float x_trans, float y_trans)
{
	// Standard - no rotation
	if (optz[o_rot] == 0)
	{
		if (ZVGPresent)
		{
			zvgFrameVector(p1.x + x_trans, p1.y + y_trans, p2.x + x_trans, p2.y + y_trans);
		}
		SDLvector(p1.x + x_trans, p1.y + y_trans, p2.x + x_trans, p2.y + y_trans, SDL_VC, SDL_VB);
	}
	// rotated LEFT (90ø CW)
	if (optz[o_rot] == 1)
	{
		if (ZVGPresent)
		{
			zvgFrameVector(p1.y + y_trans, -(p1.x + x_trans), p2.y + y_trans, -(p2.x + x_trans));
		}
		SDLvector(p1.y + y_trans, -(p1.x + x_trans), p2.y + y_trans, -(p2.x + x_trans), SDL_VC, SDL_VB);
	}
	// Rotated 180ø
	if (optz[o_rot] == 2)
	{
		if (ZVGPresent)
		{
			zvgFrameVector(-(p1.x + x_trans), -(p1.y + y_trans), -(p2.x + x_trans), -(p2.y + y_trans));
		}
		SDLvector(-(p1.x + x_trans), -(p1.y + y_trans), -(p2.x + x_trans), -(p2.y + y_trans), SDL_VC, SDL_VB);
	}
	// rotated RIGHT (90ø CCW)
	if (optz[o_rot] == 3)
	{
		if (ZVGPresent)
		{
			zvgFrameVector(-(p1.y + y_trans), (p1.x + x_trans), -(p2.y + y_trans), (p2.x + x_trans));
		}
		SDLvector(-(p1.y + y_trans), (p1.x + x_trans), -(p2.y + y_trans), (p2.x + x_trans), SDL_VC, SDL_VB);
	}
}

/********************************************************************
	Close SDL and execute MAME, restart SDL when done
********************************************************************/
void RunGame(char *gameargs)
{
	uint	err;
	char	command[80];
	
	setLEDs(0);
	CloseSDL(0);								// Close windows etc but don't quit SDL
	if (ZVGPresent)
	{
		if (optz[o_redozvg])
		{
			zvgFrameClose();					// Close the ZVG
		}
		sprintf(command, "./vmm.sh '%s %s'", gameargs, zvgargs);
	}
	else
	{
		sprintf(command, "./vmm.sh '%s'", gameargs);
	}
	printf("Launching: [%s]\n", command);
	err = system(command);
	if (optz[o_redozvg] && ZVGPresent)	// Re-open the ZVG if MAME closed it
	{
		err = zvgFrameOpen();				// initialize everything
		if (err)
		{
			zvgError( err);					// if it went wrong print error
			exit(0);								// and return to OS
		}
	}
	InitialiseSDL(0);							// re-open windows etc
}

