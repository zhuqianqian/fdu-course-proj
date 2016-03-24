EXE=bin/snf204.exe
OBJ1=bin/snfwcon.obj
OBJ2=bin/wsnfcore.obj
OBJ3=bin/fileutil.obj
ENTRY=src/snfwcon.c

CL_FLAG = /c /D "_X86_"
LINK_FLAG=/SUBSYSTEM:CONSOLE /MACHINE:X86 /INCREMENTAL:NO /NOLOGO /RELEASE

$(EXE): $(OBJ1) $(OBJ2) $(OBJ3)
	LINK $(LINK_FLAG) /out:$(EXE) $(OBJ1) $(OBJ2) $(OBJ3)

$(OBJ3): src/fileutil.c
	CL $(CL_FLAG) /Fo$(OBJ3) src/fileutil.c

$(OBJ2): src/wsnfcore.c
	CL $(CL_FLAG) /Fo$(OBJ2) src/wsnfcore.c

$(OBJ1): $(ENTRY)
	CL $(CL_FLAG) /Fo$(OBJ1) $(ENTRY)

clean:
	del /s /q /f *.obj
	
