@echo off

rem
rem @echo off disables displaying all of the code you see here in the command line
rem

echo.
echo Running cleanup procedures; press CTRL+C to exit now. Otherwise...
pause

rem
rem Move into the Backend folder, where all the magic happens
rem

setlocal

set CurDir=%CD:\=/%

cd ../Backend

rem
rem Delete generated files
rem

del program.bin
del program.elf
del output.map
del objects.list

@echo on
FOR /F "tokens=*" %%f IN (c_files) DO (del "%%~df%%~pf%%~nf.o" & del "%%~df%%~pf%%~nf.d")
@echo off

rem
rem Move into startup/ folder
rem

cd startup

rem
rem Delete compiled object files
rem

del *.o
del *.d

rem
rem Move into user source directory
rem

cd %CurDir%/src

rem
rem Delete compiled object files
rem

del *.o
del *.d

rem
rem Return to folder started from
rem

endlocal

rem
rem Display completion message and exit
rem

echo.
echo Done! All clean.
