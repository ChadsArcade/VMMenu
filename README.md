# VMMenu
## Vector Mame Menu (VMMenu) for DOS Mame with a ZVG card

 - Requires iniparser from Nicolas Devillard, at
   https://github.com/ndevilla/iniparser
 - Requires ZVG Dos SDK

Requires a DJGPP compile environment under DOS, I used the same version as used for DOS MAME circa .104

I'm no C coder, please be gentle. This was my first attempt at C.

I'v included a make file and sample compile batch file, but you may need to edit the paths in these to reflect your own environment.

Makeini can be used to generate a template ini file for VMMenu. It will query your version of Mame and generate an entry for each vector game it finds.
BiosKey can be used to display the keycode of a pressed key. Use this if you are customising the keyboard inputs

Vector Mame Memu is a frontend for Vector cabs running DOS MAME on a vector monitor via a ZVG card. It will build a list of games from your vmmenu.ini file and group them by manufacturer. The vmmenu.ini file is just a plain text file so you can edit it to change the grouping of games, remove games etc.

I'll flesh this out with more detail in due course.

<!--stackedit_data:
eyJoaXN0b3J5IjpbMTk1MTg2MDA2XX0=
-->