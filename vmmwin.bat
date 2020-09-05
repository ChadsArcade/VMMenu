::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: vmmwin.bat                                           ::
::                                                      ::
:: Chad Gray, Sep 5th 2020                              ::
::                                                      ::
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::                                                      ::
:: Helper file for VMMenu                               ::
:: Checks whether ROM is for Vectrex or MAME and runs   ::
:: the appropriate emulator                             ::
::                                                      ::
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

@ECHO OFF
set game=%~1

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Check rom name for vectrex file extension            ::
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
if %game:~-4% == .vec goto vectrex
if %game:~-4% == .bin goto vectrex
if %game:~-4% == .gam goto vectrex

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Mame game, run advmame                               ::
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:mame
echo Mame Game Detected...
echo Command line: advmame %1
advmame %1
goto end

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Vectrex game, run advmess                            ::
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:vectrex
echo Vectrex game detected...
echo Command line: advmess vectrex -cart %1
advmess vectrex -cart %1


:end

