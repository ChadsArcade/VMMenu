:: DOS VMMenu Compile
:: Chad Gray, 2011
::
:: Used when DOS compilation is carried out of a virtual XP machine
:: running on a Linux host PC.
::
:: Copies source files from Linux host machine and compiles them
::
:: Drive E: is mapped from the host machine and is the folder
:: containing the VMMenu source code
::
:: Directory structure is
::
:: VMM            (VMM src)
::   |-DOS        (DOS specific src)
::   |   \-zvg    (DOS ZVG SDK)
::   |
::   |-iniparser  (iniparser src)
::   |
::    \Linux      (Linux specific src)
::         \-zvg  (Linux ZVG SDK)
::

copy e:\VMM\dos\zvg\*.c .\DOS\zvg\
copy e:\VMM\dos\zvg\*.h .\DOS\zvg\

copy e:\VMM\dos\DOSVmm.c .\DOS\
copy e:\VMM\dos\DOSVmm.h .\DOS\

copy e:\VMM\iniparser\dictionary.h .\iniparser\
copy e:\VMM\iniparser\dictionary.c .\iniparser\
copy e:\VMM\iniparser\iniparser.h .\iniparser\
copy e:\VMM\iniparser\iniparser.c .\iniparser\

copy e:\VMM\vmmstddef.h .\
copy e:\VMM\vchars.h .\
copy e:\VMM\vchars.c .\
copy e:\VMM\gamelist.h .\
copy e:\VMM\gamelist.c .\
copy e:\VMM\vmmenu.c .\

make
pause

upx --brute vmmenu.exe
copy vmmenu.exe e:\VMM\

