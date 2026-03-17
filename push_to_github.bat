@echo off
echo Configuring git identity...
git config --global user.email "admin@authorized.lol"
git config --global user.name "authorized-lol"

echo Initializing git repository...
git init

echo Adding all files...
git add .

echo Creating commit...
set /p msg="Commit message (leave blank for 'update'): "
if "%msg%"=="" set msg=update
git commit -m "%msg%"

echo Setting branch to main...
git branch -M main

echo Setting remote origin...
git remote remove origin 2>nul
git remote add origin https://github.com/authorized-lol/C-ImGui-Example.git

echo Pushing to GitHub...
git push -u origin main --force

echo.
echo Done! https://github.com/authorized-lol/C-ImGui-Example
pause
