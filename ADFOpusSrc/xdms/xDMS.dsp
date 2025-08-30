# Microsoft Developer Studio Project File - Name="xDMS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=xDMS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xDMS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xDMS.mak" CFG="xDMS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xDMS - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "xDMS - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xDMS - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /Gi /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "xDMS - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gi /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /i "imagehlp.lib" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\xDMSdbg.lib"

!ENDIF 

# Begin Target

# Name "xDMS - Win32 Release"
# Name "xDMS - Win32 Debug"
# Begin Source File

SOURCE=.\Cdata.h
# End Source File
# Begin Source File

SOURCE=.\Crc_csum.c
# End Source File
# Begin Source File

SOURCE=.\Crc_csum.h
# End Source File
# Begin Source File

SOURCE=.\Getbits.c
# End Source File
# Begin Source File

SOURCE=.\Getbits.h
# End Source File
# Begin Source File

SOURCE=.\Maketbl.c
# End Source File
# Begin Source File

SOURCE=.\Maketbl.h
# End Source File
# Begin Source File

SOURCE=.\Pfile.c
# End Source File
# Begin Source File

SOURCE=.\Pfile.h
# End Source File
# Begin Source File

SOURCE=.\Tables.c
# End Source File
# Begin Source File

SOURCE=.\Tables.h
# End Source File
# Begin Source File

SOURCE=.\U_deep.c
# End Source File
# Begin Source File

SOURCE=.\U_deep.h
# End Source File
# Begin Source File

SOURCE=.\U_heavy.c
# End Source File
# Begin Source File

SOURCE=.\U_heavy.h
# End Source File
# Begin Source File

SOURCE=.\U_init.c
# End Source File
# Begin Source File

SOURCE=.\U_init.h
# End Source File
# Begin Source File

SOURCE=.\U_medium.c
# End Source File
# Begin Source File

SOURCE=.\U_medium.h
# End Source File
# Begin Source File

SOURCE=.\U_quick.c
# End Source File
# Begin Source File

SOURCE=.\U_quick.h
# End Source File
# Begin Source File

SOURCE=.\U_rle.c
# End Source File
# Begin Source File

SOURCE=.\U_rle.h
# End Source File
# Begin Source File

SOURCE=.\Xdms.c
# End Source File
# Begin Source File

SOURCE=.\Xdms.h
# End Source File
# End Target
# End Project
