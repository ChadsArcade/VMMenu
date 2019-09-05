# VMMenu

A menu system vector games running on a real vector monitor for DOS Mame with a ZVG card

 - Requires iniparser from Nicolas Devillard, at https://github.com/ndevilla/iniparser
 - Requires the ZVG DOS SDK
 - Requires a DJGPP compile environment under DOS, I used the same version as used for DOS MAME circa .104

You can find a walk-through of setting up such an environment here: http://mamedev.emulab.it/haze/reallyoldstuff/compile036dos.html

I'm no C coder, please be gentle. This was my first attempt at C, I'm sure my code is far from optimal and inefficient in places.
As you've probably gathered, I'm also new to Git so please be patient whilst I get to grips with things and get the files more orderly to aid compilation. 

I've included a make file and sample compile batch file, but you will need to edit the paths in these to reflect your own environment. I originally edited and compiled the files within a Windows XP environment, later I created a Linux port with common source and edit on Linux, moving to an XP Virtualbox PC which grabbed the source from the Linux host and compiled for DOS under the XP machine. The compile.bat file will reflect this environment. You could probably set up cross compiling for a simpler setup.

The Linux port has the advantage of being able to mirror the vector screen to the PC monitor, this greatly aids with development as you can test code without needing the ZVG or a vector monitor attached, and see what the colours will look like should you only have a B&W monitor. I originally developed using a vectrex as a monitor as I didn't have access to my cabs at the time, so the original colour work was done "blind".

In the Utils folder you can find some utilities:

 - **Makeini** can be used to generate a template ini file for VMMenu. It will query your version of Mame and generate an entry for each vector game it finds.
 - **BiosKey** can be used to display the keycode of a pressed key. Use this if you are customising the keyboard inputs

Vector Mame Menu will build a list of games from your vmmenu.ini file and group them by manufacturer. The vmmenu.ini file is just a plain text file so you can edit it to change the grouping of games, remove games etc.

I'll flesh this document out with more detail in due course.
