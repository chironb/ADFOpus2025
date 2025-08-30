# Microsoft Developer Studio Project File - Name="ADFOpus" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ADFOpus - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ADFOpus.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ADFOpus.mak" CFG="ADFOpus - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ADFOpus - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ADFOpus - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ADFOpus - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ADFOpus___Win32_Release"
# PROP BASE Intermediate_Dir "ADFOpus___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /Gi /GX /O2 /I "adflib/lib" /I "adflib/lib/win32" /I "xdms" /I "zlib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX"Pch.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 adflibs.lib xdms.lib zlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib imagehlp.lib /nologo /version:1.0 /subsystem:windows /machine:I386 /nodefaultlib:"libc" /libpath:"ADFLib\Bin\Win32\Release" /libpath:"xdms\Release" /libpath:"Zlib\Release"
# SUBTRACT LINK32 /verbose /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying help files...
PostBuild_Cmds=copy help\adfopus.hlp ADFOpus.hlp	copy help\adfopus.cnt ADFOpus.cnt
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ADFOpus - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ADFOpus___Win32_Debug"
# PROP BASE Intermediate_Dir "ADFOpus___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gi /GX /ZI /Od /I "adflib/lib" /I "adflib/lib/win32" /I "xdms" /I "zlib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX"Pch.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 adflibs.lib xdmsdbg.lib zlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib imagehlp.lib /nologo /version:1.0 /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd" /pdbtype:sept /libpath:"ADFLib\Bin\Win32\Debug" /libpath:"xdms\Debug" /libpath:"Zlib\Debug"
# SUBTRACT LINK32 /verbose /pdb:none /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying help files...
PostBuild_Cmds=copy help\adfopus.hlp ADFOpus.hlp	copy help\adfopus.cnt ADFOpus.cnt
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ADFOpus - Win32 Release"
# Name "ADFOpus - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\About.c
# End Source File
# Begin Source File

SOURCE=.\ADFOpus.c
# End Source File
# Begin Source File

SOURCE=.\BatchConvert.c
# End Source File
# Begin Source File

SOURCE=.\Bootblock.c
# End Source File
# Begin Source File

SOURCE=.\ChildCommon.c
# End Source File
# Begin Source File

SOURCE=.\fdi.c
# End Source File
# Begin Source File

SOURCE=.\fdi2raw.c
# End Source File
# Begin Source File

SOURCE=.\Init.c
# End Source File
# Begin Source File

SOURCE=.\Install.c
# End Source File
# Begin Source File

SOURCE=.\ListView.c
# End Source File
# Begin Source File

SOURCE=.\New.c
# End Source File
# Begin Source File

SOURCE=.\Options.c
# End Source File
# Begin Source File

SOURCE=.\Pch.c
# ADD CPP /Yc"Pch.h"
# End Source File
# Begin Source File

SOURCE=.\Properties.c
# End Source File
# Begin Source File

SOURCE=.\ShellOpen.c
# End Source File
# Begin Source File

SOURCE=.\Utils.c
# End Source File
# Begin Source File

SOURCE=.\VolInfo.c
# End Source File
# Begin Source File

SOURCE=.\VolSelect.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\About.h
# End Source File
# Begin Source File

SOURCE=.\ADFOpus.h
# End Source File
# Begin Source File

SOURCE="..\..\..\Program Files\Microsoft Visual Studio\Vc98\Include\Basetsd.h"
# End Source File
# Begin Source File

SOURCE=.\BatchConvert.h
# End Source File
# Begin Source File

SOURCE=.\Bootblock.h
# End Source File
# Begin Source File

SOURCE=.\ChildCommon.h
# End Source File
# Begin Source File

SOURCE=.\fdi.h
# End Source File
# Begin Source File

SOURCE=.\fdi2raw.h
# End Source File
# Begin Source File

SOURCE=.\Init.h
# End Source File
# Begin Source File

SOURCE=.\Install.h
# End Source File
# Begin Source File

SOURCE=.\ListView.h
# End Source File
# Begin Source File

SOURCE=.\New.h
# End Source File
# Begin Source File

SOURCE=.\Options.h
# End Source File
# Begin Source File

SOURCE=.\Pch.h
# End Source File
# Begin Source File

SOURCE=.\Properties.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\ShellOpen.h
# End Source File
# Begin Source File

SOURCE=.\types.h
# End Source File
# Begin Source File

SOURCE=.\Utils.h
# End Source File
# Begin Source File

SOURCE=.\VolInfo.h
# End Source File
# Begin Source File

SOURCE=.\VolSelect.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Images\adf.ico
# End Source File
# Begin Source File

SOURCE=.\Images\ADFOpus.ico
# End Source File
# Begin Source File

SOURCE=.\ADFOpus.rc
# End Source File
# Begin Source File

SOURCE=.\Images\adz.ico
# End Source File
# Begin Source File

SOURCE=.\Images\AmiDir.ico
# End Source File
# Begin Source File

SOURCE=.\Images\AmiFile.ico
# End Source File
# Begin Source File

SOURCE=.\Images\AmiLister.ico
# End Source File
# Begin Source File

SOURCE=.\Images\ArrowCop.cur
# End Source File
# Begin Source File

SOURCE=.\Images\dmp.ico
# End Source File
# Begin Source File

SOURCE=.\Images\dms.ico
# End Source File
# Begin Source File

SOURCE=.\Images\DriveCDROM.ico
# End Source File
# Begin Source File

SOURCE=.\Images\DriveFloppy35.ico
# End Source File
# Begin Source File

SOURCE=.\Images\DriveFloppy525.ico
# End Source File
# Begin Source File

SOURCE=.\Images\DriveHD.ico
# End Source File
# Begin Source File

SOURCE=.\Images\DriveNet.ico
# End Source File
# Begin Source File

SOURCE=.\Images\hdf.ico
# End Source File
# Begin Source File

SOURCE=.\Images\Logo.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\WinDir.ico
# End Source File
# Begin Source File

SOURCE=.\Images\WinFile.ico
# End Source File
# Begin Source File

SOURCE=.\Images\WinLister.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\Images\about.bmp
# End Source File
# Begin Source File

SOURCE=.\Help\Adfopus.hlp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Help\AdfOpus.rtf
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Images\batch.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\cascade.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\close.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\createdir.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\delete.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\disk2fdi.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\display.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\help.bmp
# End Source File
# Begin Source File

SOURCE=.\Help\history.rtf
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Images\info.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\install.bmp
# End Source File
# Begin Source File

SOURCE=.\Installers\Installers.txt
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Help\license.rtf
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Images\new.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\open.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\options.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\properti.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\rename.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\showundel.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\textviewer.bmp
# End Source File
# Begin Source File

SOURCE=.\TextViewer.c
# End Source File
# Begin Source File

SOURCE=.\TextViewer.h
# End Source File
# Begin Source File

SOURCE=.\Images\tilehor.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\tilever.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\undelete.bmp
# End Source File
# Begin Source File

SOURCE=.\Images\uponelev.bmp
# End Source File
# End Target
# End Project
