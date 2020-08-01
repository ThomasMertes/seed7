@echo off
set PATH_BACKUP=%PATH%
PATH=..\gcc\bin;%PATH%
gcc %*
PATH=%PATH_BACKUP%
