@echo off
setlocal
ECHO Patching build configuration files...
CALL tools\patch_build.bat
CALL :BUILD %CD%
GOTO :END
:BUILD
SET CEF2_DIRECTORY=%~n1
copy /Y cef2_supplement.gypi supplement.gypi
cd ..
CALL gclient runhooks --force
cd %CEF2_DIRECTORY%
del /F /Q supplement.gypi
:END
