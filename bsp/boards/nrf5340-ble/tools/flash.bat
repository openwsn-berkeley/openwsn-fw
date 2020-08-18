@echo off

rem Erase board ------------------------------------------
nrfjprog.exe --family nRF52 -e

rem Program board ----------------------------------------
nrfjprog.exe --family nRF52 --program %1

rem Reset board ------------------------------------------
nrfjprog.exe --family nRF52 -r
