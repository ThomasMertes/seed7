@echo off
set PATH_BACKUP=%PATH%
PATH=..\gcc\bin;%PATH%
ar %*
PATH=%PATH_BACKUP%
