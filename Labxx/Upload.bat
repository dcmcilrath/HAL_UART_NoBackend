@echo off

rem
rem @echo off disables displaying all of the code you see here in the command line
rem

rem
rem Move into the Backend folder, where all the magic happens
rem

setlocal

cd ../Backend

rem
rem Upload the file "program.elf" to the Discovery Board (STM32F769I-DISCO)
rem

"openocd\bin\openocd.exe" -f "scripts\st_board\stm32f769i_disco.cfg" -c "program program.elf verify reset exit"

rem
rem Return to the folder started from and exit
rem

endlocal
