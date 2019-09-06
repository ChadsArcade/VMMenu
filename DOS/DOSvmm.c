// DOS Specific functions

#include <bios.h>
#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include "vmmstddef.h"
#include "zvgFrame.h"

union		REGS in, out;
int		LEDstate=0;
int		mousexmick=0, mouseymick=0;
int		optz[15];
int		f1_press = 0, f2_press = 0, f3_press = 0, f4_press = 0;
char		zvgargs[30];
extern	int ZVGPresent;

/******************************************************************
Try to open the ZVG
*******************************************************************/
void startZVG(void)
{
	uint error;
	error = zvgFrameOpen();			// initialize everything
	if (error)
	{
		zvgError(error);		// print error
		exit(0);			// return to DOS
	}
}

/******************************************************************
Check for a keypress, return code of key (or modifier)
*******************************************************************/
int getkey(void)
{
	int key=0, shift=0;
	if (bioskey(1))
	{
 		key=bioskey(0);
		return key;
	}
	shift=bioskey(2) & 15;			// only want bits 0-3 so mask off higher 4 bits
	if (shift)	key=shift;
	// Disable autorepeat of modifiers
	if (key==FIRE)						//LCTRL
		if (f1_press==1) key=0;
		else f1_press=1;
	else f1_press=0;
	if (key==THRUST)					//L_ALT
		if (f2_press==1) key=0;
		else f2_press=1;
	else f2_press=0;
	if (key==RSHIFT)					//R_SHIFT
		if (f3_press==1) key=0;
		else f3_press=1;
	else f3_press=0;
	if (key==LSHIFT)					//L_SHIFT
		if (f4_press==1) key=0;
		else f4_press=1;
	else f4_press=0;
	return key;
}


/******************************************************************
Check whether a mouse driver is installed
*******************************************************************/
int initmouse()
{
	in.x.ax = 0;
	int86 (0X33,&in,&out);
	return out.x.ax;
}


/******************************************************************
Get the amount by which the mouse has been moved since last check
0 = return Y axis
1 = return X axis
*******************************************************************/
void mousemick()
{
	int tempaxis;
	in.x.ax = 0x0b;
	int86 (0x33, &in, &out);
	mousexmick = out.x.cx;		// read X axis
	mouseymick = out.x.dx;		// Read Y axis
	if (mousexmick > 32768) mousexmick -= 65536;
	if (mouseymick > 32768) mouseymick -= 65536;
	if (optz[o_mswapXY] == 1)	// swap x and y axes
	{
		tempaxis = mousexmick;
		mousexmick = mouseymick;
		mouseymick = tempaxis;
	}
	if (optz[o_mrevX]) mousexmick = -mousexmick;
	if (optz[o_mrevY]) mouseymick = -mouseymick;
	if (optz[o_mouse] == 1) mouseymick = 0;
}


/******************************************************************
	Get the current mouse co-ords into mpx and mpy
*******************************************************************/
void mousepos(int *mx, int *my)
{
	int x, y;
	in.x.ax = 0x03;
	int86 (0x33, &in, &out);
	x = out.x.cx;
	y = out.x.dx;
	x = (x * 1.6) - X_MAX;
	if (x > X_MAX) x = X_MAX;
	y = Y_MAX - (y * 4);
	*mx = x;
	*my = y;
}

/******************************************************************
	Write to keyboard LEDs value held in global variable LEDstate
 - only if state has changed
*******************************************************************/
void setLEDs(int value)
{
	if (LEDstate != value)
	{
		LEDstate = value;
		asm("movb $0xed, %al\n\t"
		"out %al, $0x60\n\t"
		"nop\n\t"
		"movb _LEDstate, %al\n\t"
		"out %al, $0x60");
	}
}

/******************************************************************
Get state of modifiers so we can set LEDs appropriately on exit
*******************************************************************/
int GetModifierStatus(void)
{
	int status = (bioskey(2) & 112) >> 4;	// read lock key status
	return status;
}

/******************************************************
Send the frame to the ZVG, exit if it went pear shaped
*******************************************************/
int sendframe(void)
{
	uint	err;
	tmrWaitFrame();			// wait for next frame time
	err = zvgFrameSend();	// send next frame
	if (err)
	{
		zvgError( err);
		zvgFrameClose();		// fix up all the ZVG stuff
		exit(1);
	}
	return err;
}

/******************************************************
Close everything off for a graceful exit
*******************************************************/
void ShutdownAll(void)
{
	setLEDs(GetModifierStatus());						// restore correct LED status
	if (ZVGPresent && optz[o_redozvg])
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
	zvgFrameVector(p1.x + x_trans, p1.y + y_trans, p2.x + x_trans, p2.y + y_trans);
	// rotated LEFT (90ø CW)
	if (optz[o_rot] == 1)
	zvgFrameVector(p1.y + y_trans, -(p1.x + x_trans), p2.y + y_trans, -(p2.x + x_trans));
	// Rotated 180ø
	if (optz[o_rot] == 2)
	zvgFrameVector(-(p1.x + x_trans), -(p1.y + y_trans), -(p2.x + x_trans), -(p2.y + y_trans));
	// rotated RIGHT (90ø CCW)
	if (optz[o_rot] == 3)
	zvgFrameVector(-(p1.y + y_trans), (p1.x + x_trans), -(p2.y + y_trans), (p2.x + x_trans));
}

/********************************************************************
	(Optionally) close ZVG and execute MAME, restart ZVG when done
********************************************************************/
void RunGame(char *gameargs)
{
	uint	err;
	char	command[80];
	//setLEDs(0);
	setLEDs(S_LED);

	if (ZVGPresent)
	{
		ShutdownAll();
		sprintf(command, "./vmm.bat %s %s", gameargs, zvgargs);
	}
	else
	{
		sprintf(command, "./vmm.bat %s", gameargs);
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
}

