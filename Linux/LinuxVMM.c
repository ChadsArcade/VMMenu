// Linux specific functions for the Vector Mame Menu

#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <sys/io.h>
#include <SDL.h>
#include "SDL2_gfxPrimitives.h"
#include "vmmstddef.h"
#include "LinuxVMM.h"
#include "zvgFrame.h"
#include "zvgPort.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

extern void	GetRGBfromColour(int, int*, int*, int*);					// Get R, G and B components of a passed colour

SDL_Window*	   window=NULL;
SDL_Renderer*  screenRender = NULL;
int 				WINDOW_WIDTH = 1024;
int 				WINDOW_HEIGHT = 768;
float 			WINDOW_SCALE = 1;
const 			char* WINDOW_TITLE = "Vector Mame Menu";
int 				mdx=0, mdy=0;
int				mouse_x=0, mouse_y=0;
extern			int ZVGPresent, SDL_VB, SDL_VC;
int 				optz[15];                  // array of user defined menu preferences
int				LEDstate=0;
int            keyz[11];                  // array of key press codes
extern int     mousefound;


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
	/* Set a video mode */
   window = SDL_CreateWindow( WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_BORDERLESS); // | SDL_WINDOW_FULLSCREEN); 
   screenRender = SDL_CreateRenderer(window, -1, 0);

	SDL_Event event;

   //SDL_SetWindowGrab(window, SDL_TRUE);      // SDL 2
   SDL_SetRelativeMouseMode(SDL_TRUE);

	//SDL_ShowCursor(SDL_DISABLE);
	while (SDL_PollEvent(&event)) {}				// clear event buffer
   //SDL_WarpMouseInWindow(window, 200, 200);
	//while (SDL_PollEvent(&event)) {}				// clear event buffer

	mouse_x = 0;
	mouse_y = 0;
}


/********************************************************************
	Close off SDL cleanly
********************************************************************/
void CloseSDL(int done)
{
   SDL_SetWindowGrab(window, SDL_FALSE);
	SDL_ShowCursor(SDL_ENABLE);
   SDL_DestroyWindow(window);
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
		lineRGBA(screenRender, x1/WINDOW_SCALE+(WINDOW_WIDTH/2), -y1/WINDOW_SCALE+(WINDOW_HEIGHT/2),
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
		if (!ZVGPresent) SDL_Delay(100/6);	                  // 1/60 of a sec, same as frame rate, in msecs
      SDL_RenderPresent( screenRender );                    // Flip to rendered screen
      SDL_SetRenderDrawColor(screenRender, 0, 0, 0, 255);   // Set render colour to black
      SDL_RenderClear(screenRender);                        // Clear screen
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
	Mouse types:   0 = No device
	               1 = X-Axis Spinner
	               2 = Y-Axis Spinner
	               3 = Trackball/Mouse
*******************************************************************/
void readmouse(void)
{
   if (optz[o_mouse])
   {
	   mouse_x = mdx/optz[o_msens];           // use the integer part
	   mouse_y = mdy/optz[o_msens];
	   mdx = mdx%optz[o_msens];               // retain the fractional part
	   mdy = mdy%optz[o_msens];
	   //printf("mdx: %d mdy: %d\n", mdx, mdy);

      // If spinner selected, discard the axis not in use (it might really be a mouse)
      if (optz[o_mouse] == 1) mouse_y = 0;   // Spinner which moves X-axis
      if (optz[o_mouse] == 2)                // Spinner which moves Y-axis
      {
         mouse_x = mouse_y;                  // Convert to X axis
         mouse_y = 0;                        // Discard Y axis
      }

      if (optz[o_mrevX]) mouse_x = -mouse_x;
      if (optz[o_mrevY]) mouse_y = -mouse_y;
   }
   else
   {
      mouse_x = 0;
      mouse_y = 0;
	   mdx=0;
	   mdy=0;
   }
}


/******************************************************************
Get keypress - SDL implementation. Returns scancode of pressed key
Also updates mouse x and y movements
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
            //printf("Key: %X\n", key);
//				return key;
				break;
			default:
				break;
		}
	}
	
   if (mousefound) readmouse();              // 3 Feb 2020, read every frame, ignore sample rate
   // convert mouse movement into key presses. Should really build this into the getkey function
   if (mouse_y < 0 && optz[o_mouse]==3) key = keyz[k_pgame];       // Trackball Up    = Up
   if (mouse_y > 0 && optz[o_mouse]==3) key = keyz[k_ngame];       // Trackball Down  = Down
   if (mouse_x < 0 && optz[o_mouse]==3) key = keyz[k_pclone];      // Trackball Left  = Left
   if (mouse_x > 0 && optz[o_mouse]==3) key = keyz[k_nclone];      // Trackball Right = Right
   if (mouse_x < 0 && optz[o_mouse]!=3) key = keyz[k_pgame];       // Spinner   Left  = Up
   if (mouse_x > 0 && optz[o_mouse]!=3) key = keyz[k_ngame];       // Spinner   Right = Down
	
	return key;
}


/******************************************************************
	Set Mouse Position
*******************************************************************/
void mousepos(int *mx, int *my)
{
	SDL_GetMouseState(mx, my);
   *mx = (*mx - (WINDOW_WIDTH/2 -22));
	*my = -(*my - (WINDOW_HEIGHT/2 -22));
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
void RunGame(char *gameargs, char *zvgargs)
{
	uint	err;
	char	command[80];

	setLEDs(0);
	CloseSDL(1);								// Close windows etc but don't quit SDL
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
	InitialiseSDL(1);							// re-open windows etc
}

