@echo off
set PATH_BACKUP=%PATH%
set PATH=%~dp0..\gcc\bin;%PATH%
gcc %*
set PATH=%PATH_BACKUP%
