@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Verify-Local.ps1"
if errorlevel 1 echo Offline verification failed. Keep the evidence folder.
pause
