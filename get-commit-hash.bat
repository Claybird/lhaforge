cd %~dp0
git show --format="%%x22commit-hash=%%h;%%as%%x22" --no-patch > LhaForge\commit-hash
