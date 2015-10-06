@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

msbuild.exe ../build64/voidstorm.sln /p:Configuration=Debug

