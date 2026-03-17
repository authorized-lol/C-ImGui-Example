@echo off
echo Configuring git identity...
git config --global user.email "koray.akkilic@quickmail.ink"
git config --global user.name "authorized-lol"

echo Initializing git repository...
git init

echo Adding all files...
git add .

echo Creating initial commit...
git commit -m "initial commit"

echo Setting branch to main...
git branch -M main

echo Adding remote origin...
git remote add origin https://github.com/authorized-lol/C-ImGui-Example.git

echo Pushing to GitHub...
git push -u origin main

echo.
echo Done! Check https://github.com/authorized-lol/C-ImGui-Example
pause
