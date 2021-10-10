// SDL specific functions for the Vector Mame Menu

#include <fcntl.h>
#include <SDL.h>
#include <SDL_gamecontroller.h>
#include <SDL_joystick.h>
#include "vmmstddef.h"
#include "VMM-SDL.h"
#if defined(linux) || defined(__linux)
   #include "LinuxVMM.h"
   #include <SDL2/SDL_mixer.h>
#elif defined(__WIN32__) || defined(_WIN32)
   #include "WinVMM.h"
   #include <SDL_mixer.h>
#endif
#include "zvgFrame.h"
#include <stdio.h>
#include <stdlib.h>

extern void   GetRGBfromColour(int, int*, int*, int*);               // Get R, G and B components of a passed colour

SDL_Window     *window = NULL;
SDL_Renderer   *screenRender = NULL;
int            WINDOW_WIDTH  = 800;
int            WINDOW_HEIGHT = 600;
float          WINDOW_SCALE;

#if defined(linux) || defined(__linux)
const          char* WINDOW_TITLE = "Vector Mame Menu for Linux";
#elif defined(__WIN32__) || defined(_WIN32)
const          char* WINDOW_TITLE = "Vector Mame Menu for Win32";
#endif
int            mdx=0, mdy=0;
int            MouseX=0, MouseY=0;
extern         int ZVGPresent;
int            SDL_VB, SDL_VC;            // SDL_Vector "Brightness" and "Colour"
int            optz[16];                  // array of user defined menu preferences
int            keyz[11];                  // array of key press codes
extern int     mousefound;
extern char    DVGPort[15];
uint32_t       timestart = 0, timenow, duration;
int            vector_count=0, colour_sets=0;
extern int 	   jsdeadzone;

enum vsounds
{
  sSFury,
  sNuke,
  sExplode1,  // must be at 2
  sFire1,
  sExplode2,  // must be at 4
  sFire2,
  sExplode3,  // must be at 6
  sFire3
};

//The sound effects that will be used
Mix_Chunk      *aSFury    = NULL;
Mix_Chunk      *aFire1    = NULL;
Mix_Chunk      *aFire2    = NULL;
Mix_Chunk      *aFire3    = NULL;
Mix_Chunk      *aExplode1 = NULL;
Mix_Chunk      *aExplode2 = NULL;
Mix_Chunk      *aExplode3 = NULL;
Mix_Chunk      *aNuke     = NULL;

#define		fps     60
#define		fps_ms  1000/fps

#define MAX_CONTROLLERS  8

SDL_GameController* s_controllers[MAX_CONTROLLERS];
static int s_controller_cnt;
SDL_Joystick* s_joysticks[MAX_CONTROLLERS];
static int s_joystick_cnt;

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

   #ifdef _DVGTIMER_H_
      printf(">>> DVG Hardware Version using port: %s <<<\n",DVGPort);
      ZVGPresent = 2;      
   #else
      printf(">>> ZVG Hardware Version <<<");
      ZVGPresent = 1;
   #endif
   error = zvgFrameOpen();         // initialize ZVG/DVG
   if (error)
   {
      zvgError(error);             // print error
      printf("Vector Generator hardware not found, rendering to SDL window only.\n");
      ZVGPresent = 0;
   }
   else
   {
      if (ZVGPresent != 2) 
          tmrSetFrameRate(FRAMES_PER_SEC);
      zvgFrameSetClipWin( X_MIN, Y_MIN, X_MAX, Y_MAX);
   }

   #ifdef USBDVG
      if (ZVGPresent == 2) zvgBanner();
   #endif

   InitialiseSDL(1);
}


/********************************************************************
 Initialise SDL and screen
********************************************************************/
void InitialiseSDL(int start)
{
   int guimode = 1;
   /* Initialise SDL */
   if (start)
   {
      if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) < 0)
      {
         fprintf( stderr, "Could not initialise SDL: %s\n", SDL_GetError() );
         exit( -1 );
      }
      //else printf("SDL Initialised\n");
   }

   for (int i = 0; i < SDL_NumJoysticks(); i++)
   {
      if (SDL_IsGameController(i))
      {
         //printf("Found game controller %d\n", i);
         SDL_GameController* pad = SDL_GameControllerOpen(i);
         if (pad)
         {
            int added = 0;
            if (SDL_GameControllerGetAttached(pad))
            {
               if (s_controller_cnt < MAX_CONTROLLERS)
               {
                   //printf("Attached game controller %d\n", i);
                   printf("Game controller attached: %s\n", SDL_GameControllerName(pad));
                   s_controllers[s_controller_cnt++] = pad;
                   added = 1;
               }
            }
            if (!added)
            {
               SDL_GameControllerClose(pad);
            }
         }
      }
	if (!SDL_IsGameController(i))  //Not a SDL_GameController, but still a SDL_Joystick so open it!
      {
         SDL_Joystick* joy = SDL_JoystickOpen(i);
         if (joy)
         {
            int added = 0;
            if (SDL_JoystickGetAttached(joy))
            {
               if (s_joystick_cnt < MAX_CONTROLLERS)
               {
                   //printf("Attached game controller %d\n", i);
                   printf("Joystick attached: %s\n", SDL_JoystickName(joy));
				   printf("Joystick Deadzone: %d\n", jsdeadzone);
                   s_joysticks[s_joystick_cnt++] = joy;
                   added = 1;
               }
            }
            if (!added)
            {
               SDL_JoystickClose(joy);
            }
         }
      }
   }
   if (s_controller_cnt)
   {
      SDL_GameControllerEventState(SDL_ENABLE);
   }
 if (s_joystick_cnt)
   {
      SDL_JoystickEventState(SDL_ENABLE);
   }

   // Create SDL Window
   WINDOW_WIDTH=((WINDOW_HEIGHT/3)*4); // try to make the window 4:3
   WINDOW_SCALE=(768.0/WINDOW_HEIGHT);
   window = SDL_CreateWindow( WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_HIDDEN); // | SDL_WINDOW_BORDERLESS);

   screenRender = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE); // Don't use accelerated as it is tied to the screen refresh rate
   SDL_ShowWindow(window);

   #if defined(linux) || defined(__linux)
      if (NULL == getenv("DISPLAY")) guimode = 0;
   #endif
   if (guimode) // || optz[o_dovga] || !ZVGPresent)
   {
      SDL_SetRenderDrawColor(screenRender, 0, 0, 0, 255); // Set render colour to black
      SDL_RenderClear(screenRender);                      // Clear screen
      SDL_RenderPresent(screenRender);                    // Flip to rendered screen
   }

   SDL_SetRelativeMouseMode(SDL_TRUE);
   SDL_Event event;
   while (SDL_PollEvent(&event)) {}                       // clear event buffer

   MouseX = 0;
   MouseY = 0;

   //Initialize SDL_mixer
   if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
   {
      printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
   }
   //else printf("SDL Mixer initialised\n");

   //Load sound effects
   aFire1    = Mix_LoadWAV( "VMMsnd/elim2.wav" );
   aFire2    = Mix_LoadWAV( "VMMsnd/efire.wav" );
   aFire3    = Mix_LoadWAV( "VMMsnd/fire.wav" );
   aExplode1 = Mix_LoadWAV( "VMMsnd/explode1.wav" );
   aExplode2 = Mix_LoadWAV( "VMMsnd/explode2.wav" );
   aExplode3 = Mix_LoadWAV( "VMMsnd/explode3.wav" );
   aSFury    = Mix_LoadWAV( "VMMsnd/sfury9.wav" );
   aNuke     = Mix_LoadWAV( "VMMsnd/nuke1.wav" );

   if (!aFire1)    printf( "[\033[01;33mX\033[0m] Failed to load sample ./VMMsnd/elim2.wav\n" );
   if (!aFire2)    printf( "[\033[01;33mX\033[0m] Failed to load sample ./VMMsnd/efire.wav\n" );
   if (!aFire3)    printf( "[\033[01;33mX\033[0m] Failed to load sample ./VMMsnd/fire.wav\n" );
   if (!aSFury)    printf( "[\033[01;33mX\033[0m] Failed to load sample ./VMMsnd/sfury9.wav\n" );
   if (!aExplode1) printf( "[\033[01;33mX\033[0m] Failed to load sample ./VMMsnd/explode1.wav\n" );
   if (!aExplode2) printf( "[\033[01;33mX\033[0m] Failed to load sample ./VMMsnd/explode2.wav\n" );
   if (!aExplode3) printf( "[\033[01;33mX\033[0m] Failed to load sample ./VMMsnd/explode3.wav\n" );
   if (!aNuke)     printf( "[\033[01;33mX\033[0m] Failed to load sample ./VMMsnd/nuke1.wav\n" );

   Mix_Volume(-1, optz[o_volume]);

}


/********************************************************************
	Play a sound sample
********************************************************************/
void playsound(int picksound)
{
   if (optz[o_volume] > 0)
   {
      Mix_Volume(-1, optz[o_volume]);
      switch(picksound)
      {
         case sFire1:
            Mix_PlayChannel( -1, aFire1, 0 );
            break;
         case sFire2:
            Mix_PlayChannel( -1, aFire2, 0 );
            break;
         case sFire3:
            Mix_PlayChannel( -1, aFire3, 0 );
            break;
         case sExplode1:
            Mix_PlayChannel( -1, aExplode1, 0 );
            break;
         case sExplode2:
            Mix_PlayChannel( -1, aExplode2, 0 );
            break;
         case sExplode3:
            Mix_PlayChannel( -1, aExplode3, 0 );
            break;
         case sSFury:
            Mix_PlayChannel( -1, aSFury, 0 );
            break;
         case sNuke:
            Mix_PlayChannel( -1, aNuke, 0 );
            break;
         default:
            break;
      }
   }
}


/********************************************************************
   Close off SDL cleanly
********************************************************************/
void CloseSDL(int done)
{
   for (int i = 0 ; i < s_controller_cnt ; i++)
   {
      SDL_GameControllerClose(s_controllers[i]);
      s_controllers[i] = NULL;
   }
   s_controller_cnt = 0;
   for (int i = 0 ; i < s_joystick_cnt ; i++)
   {
      SDL_JoystickClose(s_joysticks[i]);
      s_joysticks[i] = NULL;
   }
   s_joystick_cnt = 0;
  
   SDL_SetWindowGrab(window, SDL_FALSE);
   SDL_ShowCursor(SDL_ENABLE);
   SDL_DestroyWindow(window);

   Mix_FreeChunk( aFire1 );
   Mix_FreeChunk( aFire2 );
   Mix_FreeChunk( aFire3 );
   Mix_FreeChunk( aExplode1 );
   Mix_FreeChunk( aExplode2 );
   Mix_FreeChunk( aExplode3 );
   Mix_FreeChunk( aSFury );
   Mix_FreeChunk( aNuke );
   aFire1    = NULL;
   aFire2    = NULL;
   aFire3    = NULL;
   aExplode1 = NULL;
   aExplode2 = NULL;
   aExplode3 = NULL;
   aSFury    = NULL;
   aNuke     = NULL;
   Mix_CloseAudio();

   if (done)
   {
      Mix_Quit();
      SDL_Quit();
   }
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
      SDL_SetRenderDrawColor(screenRender, r*8*bright, g*8*bright, b*8*bright, 127); //SDL_ALPHA_OPAQUE);
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
      SDL_RenderPresent(screenRender);                    // Flip to rendered screen
      SDL_SetRenderDrawColor(screenRender, 0, 0, 0, 255); // Set render colour to black
      SDL_RenderClear(screenRender);                      // Clear screen
   }
   timenow = SDL_GetTicks();
   duration = (timenow-timestart);
   //printf("Start: %i\t| Time: %i\t| Loop duration: %i\t| Wait time: %i  \t| FPS ms: %i\t| VC: %i\t| CC: %i\n", timestart, timenow, duration, fps_ms-duration, fps_ms, vector_count, colour_sets);
   //if (!ZVGPresent)
   {
      if (duration < fps_ms) SDL_Delay(fps_ms-duration);
      //if ((duration < fps_ms) && ((fps_ms-duration)>0)) SDL_Delay(fps_ms-duration);
   }
   timestart=SDL_GetTicks();
   vector_count=0;
   colour_sets=0;
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
      MouseX = mdx/optz[o_msens];           // use the integer part
      MouseY = mdy/optz[o_msens];
      mdx = mdx%optz[o_msens];               // retain the fractional part
      mdy = mdy%optz[o_msens];
      //printf("mdx: %d mdy: %d\n", mdx, mdy);

      // If spinner selected, discard the axis not in use, the spinner might really be a mouse
      if (optz[o_mouse] == 1) MouseY = 0;   // Spinner which moves X-axis
      if (optz[o_mouse] == 2)                // Spinner which moves Y-axis
      {
         MouseX = MouseY;                  // Convert to X axis
         MouseY = 0;                        // Discard Y axis
      }

      if (optz[o_mrevX]) MouseX = -MouseX; // Reverse X axis if selected
      if (optz[o_mrevY]) MouseY = -MouseY; // Reverse Y axis if selected
   }
   else
   {
      MouseX = 0;                          // if we said we don't have a mouse then
      MouseY = 0;                          // set all the values to zero - there might
      mdx=0;                               // still be a mouse connected giving values
      mdy=0;                               // which we don't want to have an effect
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
      	case SDL_CONTROLLERBUTTONDOWN:
            key = 0x55550000 | event.cbutton.button;
            break;
         case SDL_MOUSEMOTION:
            mdx += event.motion.xrel;
            mdy += event.motion.yrel;
            break;
         case SDL_KEYDOWN:
            key = event.key.keysym.scancode;
            //printf("Key: %X\n", key);
            break;
         case SDL_JOYAXISMOTION:
           if (event.jaxis.axis ==0 && event.jaxis.value <-jsdeadzone) key = keyz[k_pclone]; //Joystick X-Axis left.  Use defined deadzone value.  Note:applies to all joysticks.
           if (event.jaxis.axis ==0 && event.jaxis.value >jsdeadzone) key = keyz[k_nclone];  //Joystick X-Axis right
           if (event.jaxis.axis ==1 && event.jaxis.value <-jsdeadzone) key = keyz[k_pgame];	 //Joystick Y-Axis down
           if (event.jaxis.axis ==1 && event.jaxis.value >jsdeadzone) key = keyz[k_ngame];	 //Joystick Y-Axis up
            break;
		case SDL_JOYBUTTONDOWN:
            key = 0x55550000 | (event.jbutton.which << 8) | event.jbutton.button;
            break;	
         default:
            break;
      }
   }

   if (mousefound) processmouse();              // 3 Feb 2020, read every frame, ignore sample rate

   // convert mouse movement into key presses.
   if (MouseY < 0 && optz[o_mouse]==3) key = keyz[k_pgame];       // Trackball Up    = Up
   if (MouseY > 0 && optz[o_mouse]==3) key = keyz[k_ngame];       // Trackball Down  = Down
   if (MouseX < 0 && optz[o_mouse]==3) key = keyz[k_pclone];      // Trackball Left  = Left
   if (MouseX > 0 && optz[o_mouse]==3) key = keyz[k_nclone];      // Trackball Right = Right
   if (MouseX < 0 && optz[o_mouse]!=3) key = keyz[k_pgame];       // Spinner   Left  = Up
   if (MouseX > 0 && optz[o_mouse]!=3) key = keyz[k_ngame];       // Spinner   Right = Down

   // Play sound effect based upon key pressed
   if (key == keyz[k_ngame])   playsound(sFire1);
   if (key == keyz[k_pgame])   playsound(sFire1);
   if (key == keyz[k_nclone])  playsound(sFire2);
   if (key == keyz[k_pclone])  playsound(sFire2);
   if (key == keyz[k_start])   playsound(NewScale());
   if (key == keyz[k_random])  playsound(sFire3);
   if (key == keyz[k_options]) playsound(sNuke);
   if (key == keyz[k_quit])    playsound(NewScale());
   if (key == keyz[k_menu])    playsound(sFire3);

   return key;
}


/******************************************************************
   Set Mouse Position
*******************************************************************/
void mousepos(int *mx, int *my)
{
   SDL_GetMouseState(mx, my);
   if (!ZVGPresent)
   {
      *mx = ((*mx-(WINDOW_WIDTH/2))*WINDOW_SCALE)+22;    // adjust by 22 pixels to keep the
      *my =-((*my-(WINDOW_HEIGHT/2))*WINDOW_SCALE)-22;   // pointy bit of the arrow on screen
   }
   else
   {
      *mx = (*mx - WINDOW_WIDTH/2) * 2;
      *my = (*my - WINDOW_HEIGHT/2) * -2;
   }
}


/******************************************************
Send the frame to the ZVG, exit if it went pear shaped
*******************************************************/
int sendframe(void)
{
   unsigned int   err=0;
   if (ZVGPresent)
   {
      tmrWaitForFrame();      // wait for next frame time
      //printf("Sending frame to DVG...");
      err = zvgFrameSend();     // send next frame
      //printf(" frame sent.\n");
      if (err)
      {
         zvgError( err);
         zvgFrameClose();       // fix up all the ZVG stuff
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
   #if defined(linux) || defined(__linux)
      setLEDs(8);
   #else
      setLEDs(0);                                 // restore LED status
   #endif
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
   colour_sets++;     // For debug, count the number of setcolour calls/frame
}


/*******************************************************************
 Draw a vector - pass the start, end points, and the x and y offsets
 Uses global rotation variable to determine orientation
********************************************************************/
void drawvector(point p1, point p2, float x_trans, float y_trans)
{
   // Standard - no rotation
   vector_count++;    // For debug, count the number of vectors drawn/frame
   if (optz[o_rot] == 0)
   {
      if (ZVGPresent)
      {
         zvgFrameVector(p1.x + x_trans, p1.y + y_trans, p2.x + x_trans, p2.y + y_trans);
      }
      SDLvector(p1.x + x_trans, p1.y + y_trans, p2.x + x_trans, p2.y + y_trans, SDL_VC, SDL_VB);
   }
   // rotated LEFT (90° CW)
   if (optz[o_rot] == 1)
   {
      if (ZVGPresent)
      {
         zvgFrameVector(p1.y + y_trans, -(p1.x + x_trans), p2.y + y_trans, -(p2.x + x_trans));
      }
      SDLvector(p1.y + y_trans, -(p1.x + x_trans), p2.y + y_trans, -(p2.x + x_trans), SDL_VC, SDL_VB);
   }
   // Rotated 180°
   if (optz[o_rot] == 2)
   {
      if (ZVGPresent)
      {
         zvgFrameVector(-(p1.x + x_trans), -(p1.y + y_trans), -(p2.x + x_trans), -(p2.y + y_trans));
      }
      SDLvector(-(p1.x + x_trans), -(p1.y + y_trans), -(p2.x + x_trans), -(p2.y + y_trans), SDL_VC, SDL_VB);
   }
   // rotated RIGHT (90° CCW)
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
   unsigned int   err;
   char   command[200];

   setLEDs(0);
   CloseSDL(0);                        // Close windows etc but don't quit SDL
   if (ZVGPresent)
   {
      //if (optz[o_redozvg])
      //{
         zvgFrameClose();              // Close the ZVG
      //}
   }
   #if defined(linux) || defined(__linux)
      sprintf(command, "./vmm.sh \"%s\"", gameargs);
   #elif defined(__WIN32__) || defined(_WIN32)
      sprintf(command, "vmmwin.bat \"%s\"", gameargs);
   #endif
   printf("Launching: [%s]\n", command);
   err = system(command);
   //if (optz[o_redozvg] && ZVGPresent)  // Re-open the ZVG if MAME closed it
   if (ZVGPresent)                     // Re-open the ZVG if MAME closed it
   {
      err = zvgFrameOpen();            // initialize everything
      if (err)
      {
         zvgError( err);               // if it went wrong print error
         exit(0);                      // and return to OS
      }
   }
   InitialiseSDL(1);                   // re-open windows etc
}

