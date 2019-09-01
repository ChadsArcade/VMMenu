# VMMenu
Vector Mame Menu for ZVG

Requires iniparser from Nicolas Devillard, at https://github.com/ndevilla/iniparser

Requires ZVG Dos SDK
Requires a DJGPP compile environment under DOS, I used the same version as used for DOS MAME circa .104

I'm no C coder, please be gentle. This was my first attempt at C.

I'v included a make file and sample compile batch file, but you may need to edit the paths in these to reflect your own environment.

Makeini can be used to generate a template ini file for VMMenu. It will query your version of Mame and generate an entry for each vector game it finds.
BiosKey can be used to display the keycode of a pressed key. Use this if you are customising the keyboard inputs
