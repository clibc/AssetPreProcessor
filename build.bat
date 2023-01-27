@echo off
pushd _Build
      set CommonFlags= /Od /Oi /GR- /EHa- /FC /nologo /EHsc /Zi /W4 /wd4100 /wd4189 /wd4505
      cl %CommonFlags% ..\Source\Main.cpp /link /incremental:no user32.lib gdi32.lib /SUBSYSTEM:CONSOLE
popd
