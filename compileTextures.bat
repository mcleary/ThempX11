@ECHO Off
SETLOCAL ENABLEDELAYEDEXPANSION 
SET "sourcedir=resources\textures"
SET "destdir=data\textures"

FOR /f "delims=" %%a IN ('xcopy /y /L /s "%sourcedir%\*"') DO (
 SET "destfile=%%a"
 SET "destfile=!destfile:*%sourcedir%=%destdir%!"
 IF /i "%%a" neq "!destfile!" (
  FOR %%m IN ("!destfile!") DO IF NOT EXIST "%%~dpm%%~na.dds" (
   MD "%%~dpm" 2>nul
   "bin\ThempImageParser.exe" "%%a" "%%~dpm%%~na.dds"
  )
 )
)
pause
GOTO :EOF