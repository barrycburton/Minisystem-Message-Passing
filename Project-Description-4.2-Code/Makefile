# Makefile for minisystem on x86/XP

# You probably do not need to modify this, unless you are adding extra files.

CC = cl.exe
LINK = link.exe

WINSOCKLIB = ws2_32.lib

#VS.NET 2005
VISUALSTUDIO = C:\Program Files\Microsoft Visual Studio 8\VC

SYSLIBPATH = "$(VISUALSTUDIO)\lib\\"

PLATFORMLIBPATH = "$(VISUALSTUDIO)\PlatformSDK\lib\\"

CFLAGS = /nologo /MTd /W3 /Gm /RTC1 /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"minisystem.pch" /Fo"" /Fd"" /FD /EHsc /c 

LFLAGS = /nologo /subsystem:console /incremental:no /pdb:"minisystem.pdb" /debug /machine:I386 /out:"minisystem.exe" /LIBPATH:$(SYSLIBPATH) /LIBPATH:$(PLATFORMLIBPATH)

LIB = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(WINSOCKLIB)





# change this to the name of the file you want to link with minisystem, 
# dropping the ".c": so to use "sieve.c", change to "MAIN = sieve".




MAIN = app_mp_buffer






OBJ = minithread.obj \
	queue.obj \
	synch.obj \
	multilevel_queue.obj \
	priority_queue.obj \
	alarm.obj \
	directory.obj \
	minimsg.obj \
	$(MAIN).obj
		
		
SYSTEMOBJ = interrupts.obj \
	network.obj \
	machineprimitives_x86.obj \
	machineprimitives.obj \

	
	
all: minisystem.exe

.c.obj:
	$(CC) $(CFLAGS) $<

minisystem.exe: start.obj end.obj $(OBJ) $(SYSTEMOBJ)
	$(LINK) $(LFLAGS) $(LIB) $(SYSTEMOBJ) start.obj $(OBJ) end.obj $(LFLAGS)

clean:
	-@del /F /Q *.obj
	-@del /F /Q minisystem.pch minisystem.pdb 
	-@del /F /Q minisystem.exe

depend: 
	gcc -MM *.c 2>/dev/null | sed -e "s/\.o/.obj/" > depend
