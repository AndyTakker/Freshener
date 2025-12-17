@echo off
@chcp 1251
set /p name=Enter comment:
setlocal
rem Задаём значение по умолчанию
set "COMMENT="QuickAutoUpdate""

rem Проверяем, передан ли параметр
if not "%name%"=="" (
    set "COMMENT=%name%"
)

echo Используется комментарий: %COMMENT%

@echo ===== Fetch from github =====
git fetch
pause
@echo ===== Merge from github =====
git merge
pause
@echo ===== Add files =====
git add .
pause
@echo ===== Commit/push to github =====
git commit -a -m "%COMMENT%"
REM git push --set-upstream origin main
git push
pause
