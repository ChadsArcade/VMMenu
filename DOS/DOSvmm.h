/**************************************
DOS-vmmenu.h
Chad Gray, 18 Jun 2011
DOS Specific Function declarations
**************************************/

#ifndef _DOSVMM_H_
#define _DOSVMM_H_

void	startZVG(void);
int	getkey(void);
int	initmouse(void);
void	mousemick(void);
void	setLEDs(int);
int	sendframe(void);
void	ShutdownAll(void);
void	drawvector(point, point, float, float);					// draw a vector between 2 points
void	RunGame(char*, char*);
void	mousepos(int*, int*);
void	KBLEDs(void);
int	GetModifierStatus(void);

#endif

