/**************************************
VMM-SDL.h
Chad Gray, 18 Jun 2011
SDL Specific Function declarations
**************************************/

#ifndef _VMM_SDL_H_
#define _VMM_SDL_H_

void  startZVG(void);                                   // Start up the Vector Generator
int   getkey(void);                                     // Read keyboard and mouse
int   initmouse(void);                                  // initialise the mouse
void  processmouse(void);                               // Process mouse/spinner movement
int   sendframe(void);                                  // Send a frame to the VG and/or SDL
void  ShutdownAll(void);                                // Shutdown the VG and SDL
void  drawvector(point, point, float, float);           // draw a vector between 2 points
void	RunGame(char*, char*);                            // Generate command to run a game
void  FrameSendSDL(void);                               // Send a frame to the SDL surface
void  SDLvector(float, float, float, float, int, int);  // Draw a vector on the SDL surface
void  InitialiseSDL(int);                               // Start up SDL
void  CloseSDL(int);                                    // Close down SDL
void  mousepos(int*, int*);                             // Mouse position
void  setcolour(int, int);                              // set colour and brightness of next vector
void  playsound(int);                                   // Play a sound effect

#endif

