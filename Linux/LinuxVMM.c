// Linux specific functions for the Vector Mame Menu

#include "vmmstddef.h"
#include "LinuxVMM.h"
#include <sys/ioctl.h>
#include <sys/kd.h>
#include <fcntl.h>
#include <unistd.h>

int LEDstate=0;

/******************************************************************
   Write to keyboard LEDs value held in global variable LEDstate
   only if state has changed
   Unfortunately the use of ioctl requires root access,
   or (apparently) the capability "CAP_SYS_TTY_CONFIG"
*******************************************************************/
void setLEDs(int leds)
{
   int fd;
   if (LEDstate != leds)
   {
      if ((fd = open("/dev/console", O_NOCTTY)) != -1)
      {
         ioctl(fd, KDSETLED, leds);
         close(fd);
      }
      LEDstate = leds;
   }
}
