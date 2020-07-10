// Linux specific functions for the Vector Mame Menu

//#include <sys/ioctl.h>
//#include <fcntl.h>
//#include <linux/kd.h>
//#include <sys/io.h>
#include "vmmstddef.h"
#include "LinuxVMM.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

int            LEDstate=0;

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
