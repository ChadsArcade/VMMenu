// SDL specific functions for the Vector Mame Menu

#include <fcntl.h>
#include <SDL.h>
#include "vmmstddef.h"
#include "VMM-SDL.h"
#if defined(linux) || defined(__linux)
   #include "LinuxVMM.h"
#elif defined(__WIN32__) || defined(_WIN32)
   #include "WinVMM.h"
#endif
#include "zvgFrame.h"
#include <stdio.h>
#include <stdlib.h>

extern void	GetRGBfromColour(int, int*, int*, int*);					// Get R, G and B components of a passed colour

SDL_Window     *window = NULL;
SDL_Renderer   *screenRender = NULL;
int            WINDOW_WIDTH  = 800;
int            WINDOW_HEIGHT = 600;
#if defined(linux) || defined(__linux)
const          char* WINDOW_TITLE = "Vector Mame Menu for Linux";
#elif defined(__WIN32__) || defined(_WIN32)
const          char* WINDOW_TITLE = "Vector Mame Menu for Win32";
#endif
int            mdx=0, mdy=0;
int            mouse_x=0, mouse_y=0;
extern         int ZVGPresent;
int            SDL_VB, SDL_VC;            // SDL_Vector "Brightness" and "Colour"
int            optz[15];                  // array of user defined menu preferences
int            keyz[11];                  // array of key press codes
extern int     mousefound;
extern char    DVPort[15];

/******************************************************************
	Start up the DVG if poss and use SDL if necessary
*******************************************************************/
void startZVG(void)
{
	unsigned int error;
#if DEBUG	
	printf("Key UP:       0x%04x\n", UP);
	printf("Key DOWN:     0x%04x\n", DOWN);
	printf("Key LEFT:     0x%04x\n", LEFT);
	printf("Key RIGHT:    0x%04x\n", RIGHT);
	printf("Key SETTINGS: 0x%04x\n", GRAVE);
	printf("Key ESCAPE:   0x%04x\n", ESC);
	printf("Key CREDIT:   0x%04x\n", CREDIT);
	printf("Key START1:   0x%04x\n", START1);
	printf("Key START2:   0x%04x\n", START2);
	printf("Key FIRE:     0x%04x\n", FIRE);
	printf("Key THRUST:   0x%04x\n", THRUST);
	printf("Key HYPSPACE: 0x%04x\n", HYPSPACE);
	printf("Key RSHIFT:   0x%04x\n", RSHIFT);
	printf("Key LSHIFT:   0x%04x\n", LSHIFT);
#endif
	
	InitialiseSDL(1);
   #ifdef _DVGTIMER_H_
      printf("\n>>> DVG Hardware Version <<<\n");
      printf(">>> DVGPort is \"%s\"\n\n", DVPort);
   #else
      printf("\n>>> ZVG Hardware Version <<<\n");
   #endif
	error = zvgFrameOpen();			// initialize ZVG
	if (error)
	{	zvgError( error);				// print error
		printf("Vector Generator hardware not found, rendering to SDL window only.\n");
		ZVGPresent = 0;
	}
	else
   {
      tmrSetFrameRate( FRAMES_PER_SEC);
      zvgFrameSetClipWin( X_MIN, Y_MIN, X_MAX, Y_MAX);
   }
//   #if DEBUG
//      //print out a ZVG banner, indicating version etc.
//      zvgBanner( ZvgSpeeds, &ZvgID);
//      printf("\n\n");
//   #endif

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
                              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN); // || SDL_WINDOW_INPUT_GRABBED);

   screenRender = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
   //screenRender = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

   SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_Event event;
	while (SDL_PollEvent(&event)) {}				// clear event buffer

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
   float WINDOW_SCALE = (1024.0/WINDOW_WIDTH);

   if (optz[o_dovga] || !ZVGPresent)
   {
      if (clr > 7) clr = vwhite;
      if (bright > 31) bright = 31;
      GetRGBfromColour(clr, &r, &g, &b);
      SDL_SetRenderDrawColor(screenRender, r*8*bright, g*8*bright, b*8*bright, SDL_ALPHA_OPAQUE);
      SDL_RenderDrawLine(screenRender, x1/WINDOW_SCALE+(WINDOW_WIDTH/2), -y1/WINDOW_SCALE+(WINDOW_HEIGHT/2),
                                       x2/WINDOW_SCALE+(WINDOW_WIDTH/2), -y2/WINDOW_SCALE+(WINDOW_HEIGHT/2));
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
	return 1;                                 // Let's just say yes...
}

/******************************************************************
Get the amount by which the mouse has been moved since last check
	deltas are read and accumulated by keyboard event function
	Mouse types:   0 = No device
	               1 = X-Axis Spinner
	               2 = Y-Axis Spinner
	               3 = Trackball/Mouse
*******************************************************************/
void processmouse(void)
{
   if (optz[o_mouse])
   {
	   mouse_x = mdx/optz[o_msens];           // use the integer part
	   mouse_y = mdy/optz[o_msens];
	   mdx = mdx%optz[o_msens];               // retain the fractional part
	   mdy = mdy%optz[o_msens];
	   //printf("mdx: %d mdy: %d\n", mdx, mdy);

      // If spinner selected, discard the axis not in use, the spinner might really be a mouse
      if (optz[o_mouse] == 1) mouse_y = 0;   // Spinner which moves X-axis
      if (optz[o_mouse] == 2)                // Spinner which moves Y-axis
      {
         mouse_x = mouse_y;                  // Convert to X axis
         mouse_y = 0;                        // Discard Y axis
      }

      if (optz[o_mrevX]) mouse_x = -mouse_x; // Reverse X axis if selected
      if (optz[o_mrevY]) mouse_y = -mouse_y; // Reverse Y axis if selected
   }
   else
   {
      mouse_x = 0;                           // if we said we don't have a mouse then
      mouse_y = 0;                           // set all the values to zero - there might
	   mdx=0;                                 // still be a mouse connected giving values
	   mdy=0;                                 // which we don't want to have an effect
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
				break;
			default:
				break;
		}
	}

   if (mousefound) processmouse();              // 3 Feb 2020, read every frame, ignore sample rate
   // convert mouse movement into key presses. Now built into the getkey function
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


/******************************************************
Send the frame to the ZVG, exit if it went pear shaped
*******************************************************/
int sendframe(void)
{
	unsigned int	err=0;
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
	setLEDs(0);                                    // restore LED status
	if (ZVGPresent)
	{
		zvgFrameClose();                            // fix up all the ZVG stuff
	}
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
   SDL_VC = clr;      // a bit hacky, it was a late addition.
   SDL_VB = bright;   // should pass as parameters to draw functions
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
	unsigned int	err;
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
