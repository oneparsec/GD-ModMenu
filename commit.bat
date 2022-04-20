@echo off


set /p "var=Message: "
git commit -m "%var%" src/dllmain.cpp
git push origin main