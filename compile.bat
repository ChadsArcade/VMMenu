copy e:\LinuxMenu\zvg\dos\*.c ..\zvg\
copy e:\LinuxMenu\zvg\dos\*.h ..\zvg\

copy e:\LinuxMenu\zvg\vmmstddef.h ..\zvg\
copy e:\LinuxMenu\zvg\vchars.h ..\zvg\
copy e:\LinuxMenu\zvg\vchars.c ..\zvg\
copy e:\LinuxMenu\zvg\gamelist.h ..\zvg\
copy e:\LinuxMenu\zvg\gamelist.c ..\zvg\

copy e:\LinuxMenu\zvg\dictionary.h ..\zvg\
copy e:\LinuxMenu\zvg\dictionary.c ..\zvg\
copy e:\LinuxMenu\zvg\iniparser.h ..\zvg\
copy e:\LinuxMenu\zvg\iniparser.c ..\zvg\

copy e:\LinuxMenu\vmmenu\vmmenu.c .

make
pause

upx --brute vmmenu.exe
copy vmmenu.exe e:\LinuxMenu\vmmenu\

