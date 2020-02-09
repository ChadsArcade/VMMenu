/**************************************
LinuxVMM.h
Chad Gray, 18 Jun 2011
Linux Specific Function declarations
**************************************/

#ifndef _LINUXVMM_H_
#define _LINUXVMM_H_

void	startZVG(void);
int	getkey(void);
int	initmouse(void);
void	readmouse(void);
void	setLEDs(int);
int	sendframe(void);
void	ShutdownAll(void);
void	drawvector(point, point, float, float);					// draw a vector between 2 points
void	RunGame(char*, char*);
void	FrameSendSDL(void);
void	SDLvector(float, float, float, float, int, int);
void	InitialiseSDL(int);
void	CloseSDL(int);
char*	itoa(int, char*, int);
void mousepos(int*, int*);

#endif

