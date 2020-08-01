@echo off
set PATH_BACKUP=%PATH%
PATH=%~dp0..\gcc\bin;%PATH%
gcc %*
PATH=%PATH_BACKUP%
