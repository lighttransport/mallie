rem -- config --
rem set target=vs2010
rem set buildtype=release

set target=vs2012
set buildtype=release
set myplatform=x64

tools\\windows\\premake4.exe --with-sdl --platform=%myplatform% %target% %buildtype%
