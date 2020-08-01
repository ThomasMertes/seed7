@echo off
set PATH_BACKUP=%PATH%
set PATH=%~dp0..\gcc\bin;%PATH%
ar %*
set PATH=%PATH_BACKUP%
