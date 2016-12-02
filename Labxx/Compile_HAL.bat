@echo off

rem
rem @echo off disables displaying all of the code you see here in the command line
rem

rem
rem Allow local variables to be created (so that this script doesn't mess with
rem system variables). Also, ENABLEDELAYEDEXPANSION allows local variables to
rem be modified
rem

setlocal ENABLEDELAYEDEXPANSION

rem CurDir is DOS path
rem CurDir2 is DOS path with /
rem CurDir3 is Unix path with \ before spaces

set CurDir=%CD%
set CurDir2=%CurDir:\=/%
set CurDir3=%CurDir2: =\ %

rem
rem Move into the Backend folder, where all the magic happens
rem

cd ../Backend

rem
rem First things first, delete the objects list to rebuild it later
rem

del objects.list

rem
rem Create the HFILES variable, which contains the massive set of includes (-I)
rem needed by GCC.
rem
rem Two of the include folders are always included, and they
rem are %CurDir%/inc/ (the user-header directory) and Backend/startup/ (what the DISCO needs
rem for all HAL projects).
rem

set HFILES="%CurDir%\inc\" -I"startup\"

rem
rem Loop through the h_files file and turn each include directory into -I strings
rem

FOR /F "tokens=*" %%h IN (h_files) DO set HFILES=!HFILES! -I"%%h"

rem
rem Windows uses backslashes. GCC and *nix-based things expect forward slashes.
rem This converts backslashes into forward slashes on Windows.
rem

set HFILES=%HFILES:\=/%

rem
rem These are useful for debugging this script, namely to make sure you aren't
rem missing any include directories.
rem

rem echo !HFILES!
rem pause

rem
rem Loop through and compile the backend .c files, which are listed in c_files
rem

@echo on
FOR /F "tokens=*" %%f IN (c_files) DO "gcc-arm-none-eabi-5_4-2016q2-20160622-win32\bin\arm-none-eabi-gcc.exe" -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -DUSE_HAL_DRIVER -DSTM32F769xx -DUSE_STM32F769I_DISCO -I%HFILES% -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"%%~df%%~pf%%~nf.d" -MT"%%~df%%~pf%%~nf.o" -o "%%~df%%~pf%%~nf.o" "%%~df%%~pf%%~nf.c"
@echo off

rem
rem Compile the .c files in the startup/ folder
rem

@echo on
FOR %%f IN ("startup/*.c") DO "gcc-arm-none-eabi-5_4-2016q2-20160622-win32\bin\arm-none-eabi-gcc.exe" -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -DUSE_HAL_DRIVER -DSTM32F769xx -DUSE_STM32F769I_DISCO -I%HFILES% -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"startup/%%~nf.d" -MT"startup/%%~nf.o" -o "startup/%%~nf.o" "startup/%%~nf.c"
@echo off

rem
rem Compile the .s files in the startup folder (Assembly files needed to
rem initialize the DISCO)
rem

@echo on
FOR %%f IN ("startup/*.s") DO "gcc-arm-none-eabi-5_4-2016q2-20160622-win32\bin\arm-none-eabi-as.exe" -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -I"%CurDir2%/inc/" -g -o "startup/%%~nf.o" "startup/%%~nf.s"
@echo off

rem
rem Compile user .c files
rem

@echo on
FOR %%f IN ("%CurDir2%/src/*.c") DO "gcc-arm-none-eabi-5_4-2016q2-20160622-win32\bin\arm-none-eabi-gcc.exe" -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -DUSE_HAL_DRIVER -DSTM32F769xx -DUSE_STM32F769I_DISCO -I%HFILES% -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"%CurDir2%/src/%%~nf.d" -MT"%CurDir2%/src/%%~nf.o" -o "%CurDir2%/src/%%~nf.o" "%CurDir2%/src/%%~nf.c"
@echo off

rem
rem Create OBJECTS variable, whose sole purpose is to allow conversion of
rem backslashes into forward slashes in Windows
rem

set OBJECTS=

rem
rem Create the objects.list file, which contains properly-formatted (i.e. has
rem forward slashes) locations of compiled backend .o files
rem

FOR /F "tokens=*" %%f IN (c_files) DO (set OBJECTS="%%~df%%~pf%%~nf.o" & set OBJECTS=!OBJECTS:\=/! & set OBJECTS=!OBJECTS: =\ ! & set OBJECTS=!OBJECTS:"\ \ ="! & echo !OBJECTS! >> objects.list)

rem
rem Add compiled .o files from the startup/ directory to objects.list
rem

FOR %%f IN ("startup/*.o") DO echo "startup/%%f" >> objects.list

rem
rem Add compiled user .o files to objects.list
rem

FOR %%f IN ("%CurDir2%/src/*.o") DO echo "%CurDir3%/src/%%~nxf" >> objects.list

rem
rem Link all the object files using a linker script (in the Linker directory)
rem and all the objects in objects.list to generate the output binary, which is
rem called "program.elf"
rem

@echo on
"gcc-arm-none-eabi-5_4-2016q2-20160622-win32\bin\arm-none-eabi-gcc.exe" -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -specs=nosys.specs -specs=nano.specs -T"Linker\LinkerScript.ld" -Wl,-Map=output.map -Wl,--gc-sections -lm -o "program.elf" @"objects.list"
@echo off

rem
rem Make a .bin version of program.elf and output the program size
rem

echo.
echo Generating binary and Printing size information:
echo.
"gcc-arm-none-eabi-5_4-2016q2-20160622-win32\bin\arm-none-eabi-objcopy" -O binary "program.elf" "program.bin"
"gcc-arm-none-eabi-5_4-2016q2-20160622-win32\bin\arm-none-eabi-size" "program.elf"
echo.

rem
rem Return to the folder started from and exit
rem
rem No more need for local variables
rem

endlocal

echo **********************************************************
