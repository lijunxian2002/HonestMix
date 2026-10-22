@echo off
setlocal
set "HM_ROOT=%~dp0"
if not exist "%HM_ROOT%tmp" mkdir "%HM_ROOT%tmp"
set "TEMP=%HM_ROOT%tmp"
set "TMP=%HM_ROOT%tmp"
if not defined VSINSTALLDIR (
 for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "HM_VS=%%i"
) else (set "HM_VS=%VSINSTALLDIR%")
if not defined HM_VS exit /b 1
call "%HM_VS%\Common7\Tools\VsDevCmd.bat" -no_logo -host_arch=x64 -arch=x64
if errorlevel 1 exit /b 1
set "PATH=%HM_VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%HM_VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
set "HM_JUCE_OPTIONS="
if defined HM_JUCE set HM_JUCE_OPTIONS="-DHM_JUCE=%HM_JUCE%"
cmake -S "%HM_ROOT%." -B "%HM_ROOT%build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DHM_BUILD_PEAK_PROBE=ON %HM_JUCE_OPTIONS%
if errorlevel 1 exit /b 1
if "%~1"=="--configure-only" exit /b 0
cmake --build "%HM_ROOT%build" --target HonestMixHeadphone_VST3 HonestMixHeadphone_Standalone HeadphoneTests FidelityCheck ResponseContractTests TempoRecallTests PeakTelemetryProbe -j 2
exit /b %errorlevel%

