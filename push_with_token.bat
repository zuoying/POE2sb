@echo off
echo === Pushing Changes to GitHub with Token ===
echo Date: %date% %time%
echo.
echo This script uses GitHub Personal Access Token for authentication.
echo You need to create a token at: https://github.com/settings/tokens
echo.
set /p GITHUB_TOKEN="Enter your GitHub Personal Access Token: "

echo.
echo Setting up remote URL with token...
git remote set-url origin https://%GITHUB_TOKEN%@github.com/zuoying/POE2sb.git

echo.
echo Pushing changes...
git push origin main

echo.
echo Resetting remote URL...
git remote set-url origin https://github.com/zuoying/POE2sb.git

echo.
echo === Push Completed ===
echo.
pause