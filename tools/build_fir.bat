@echo off
call "D:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
cl /nologo /O2 /std:c++17 /Fe:C:\Users\Administrator\source\repos\HonestMix\tools\export_fir.exe C:\Users\Administrator\source\repos\HonestMix\tools\export_fir.cpp
C:\Users\Administrator\source\repos\HonestMix\tools\export_fir.exe > C:\Users\Administrator\source\repos\HonestMix\Source\dsp\fir_data.h
echo DONE
