@rem msbuild spawner_win32.vcxproj /t:clean /p:Configuration=Release /p:Platform=Win32
@rem msbuild spawner_win32.vcxproj /p:Configuration=Release /p:Platform=Win32
@rem copy Release\spawner_win32.exe ..\tilemill\TileMill.exe

rd /q /s Debug
rd /q /s Release
del build.sln
python gyp/gyp build.gyp --depth=. -f msvs -G msvs_version=2010
msbuild build.sln
rem msbuild spawner_win32.vcxproj
copy Default\TileMill.exe TileMill.exe
