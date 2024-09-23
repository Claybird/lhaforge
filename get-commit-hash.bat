cd %~dp0
git show --format="%%x22commit-hash=%%h;%%as%%x22" --no-patch > LhaForge\commit-hash || call :onError
exit /b

:onError
echo "commit-hash=-------;9999-99-99" > LhaForge\commit-hash
exit /b
