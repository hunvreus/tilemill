@rem msbuild spawner_win32.vcxproj /t:clean /p:Configuration=Release /p:Platform=Win32
@rem msbuild spawner_win32.vcxproj /p:Configuration=Release /p:Platform=Win32
@rem copy Release\spawner_win32.exe ..\tilemill\TileMill.exe

rd /q /s Debug
rd /q /s Release
msbuild spawner_win32.vcxproj
copy Debug\spawner_win32.exe TileMill.exe