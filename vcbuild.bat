rem -- config --
rem set target=vs2010
rem set buildtype=release

set target=vs2013
set buildtype=release
set myplatform=x64

tools\\windows\\premake5.exe --with-sdl %target% %buildtype%
