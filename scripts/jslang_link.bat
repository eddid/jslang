@call "c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
REM link.exe jslang.obj jslang.dir\Debug\jsextern.obj jslang.dir\Debug\allinone_for_link.obj "msvcrtd.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "comdlg32.lib" "advapi32.lib" /DEBUG /SUBSYSTEM:CONSOLE /STACK:"10000000" /OUT:jslang.exe
link.exe %1 ..\build\jslang.dir\Debug\jsextern.obj ..\build\jslang.dir\Debug\allinone_for_link.obj "..\build\Debug\pcre.lib" "..\build\Debug\pthread.lib" /DEBUG /SUBSYSTEM:CONSOLE /STACK:"10000000" /OUT:%2
@pause