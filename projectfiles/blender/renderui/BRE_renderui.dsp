# Microsoft Developer Studio Project File - Name="BRE_renderui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=BRE_renderui - Win32 Profile
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "BRE_renderui.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "BRE_renderui.mak" CFG="BRE_renderui - Win32 Profile"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BRE_renderui - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "BRE_renderui - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "BRE_renderui - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "BRE_renderui - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "..\..\..\..\obj\windows\blender\renderui"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\lib\windows\guardedalloc\include" /I "..\..\..\source\blender\renderui" /I "..\..\..\source\blender" /I "..\..\..\source\blender\bpython\include" /I "..\..\..\source\blender\misc" /I "..\..\..\source\blender\renderconverter" /I "..\..\..\source\blender\include" /I "..\..\..\source\blender\blenkernel" /I "..\..\..\source\blender\makesdna" /I "..\..\..\source\blender\blenlib" /I "..\..\..\source\blender\render\extern\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /J /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\obj\windows\blender\renderui\BRE_renderui.lib"

!ELSEIF  "$(CFG)" == "BRE_renderui - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "..\..\..\..\obj\windows\blender\renderui"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\lib\windows\guardedalloc\include" /I "..\..\..\source\blender\renderui" /I "..\..\..\source\blender" /I "..\..\..\source\blender\bpython\include" /I "..\..\..\source\blender\misc" /I "..\..\..\source\blender\renderconverter" /I "..\..\..\source\blender\include" /I "..\..\..\source\blender\blenkernel" /I "..\..\..\source\blender\makesdna" /I "..\..\..\source\blender\blenlib" /I "..\..\..\source\blender\render\extern\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /J /FD /GZ /c
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\obj\windows\blender\renderui\debug\BRE_renderui.lib"

!ELSEIF  "$(CFG)" == "BRE_renderui - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "BRE_renderui___Win32_Profile"
# PROP BASE Intermediate_Dir "BRE_renderui___Win32_Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "BRE_renderui___Win32_Profile"
# PROP Intermediate_Dir "BRE_renderui___Win32_Profile"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\lib\windows\guardedalloc\include" /I "..\..\..\source\blender\renderui" /I "..\..\..\source\blender" /I "..\..\..\source\blender\bpython\include" /I "..\..\..\source\blender\misc" /I "..\..\..\source\blender\renderconverter" /I "..\..\..\source\blender\include" /I "..\..\..\source\blender\blenkernel" /I "..\..\..\source\blender\makesdna" /I "..\..\..\source\blender\blenlib" /I "..\..\..\source\blender\render\extern\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /I "..\..\..\lib\windows\guardedalloc\include" /I "..\..\..\source\blender\renderui" /I "..\..\..\source\blender" /I "..\..\..\source\blender\bpython\include" /I "..\..\..\source\blender\misc" /I "..\..\..\source\blender\renderconverter" /I "..\..\..\source\blender\include" /I "..\..\..\source\blender\blenkernel" /I "..\..\..\source\blender\makesdna" /I "..\..\..\source\blender\blenlib" /I "..\..\..\source\blender\render\extern\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /J /FD /GZ /c
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\..\obj\windows\blender\renderui\debug\BRE_renderui.lib"
# ADD LIB32 /nologo /out:"..\..\..\..\obj\windows\blender\renderui\debug\BRE_renderui.lib"

!ENDIF 

# Begin Target

# Name "BRE_renderui - Win32 Release"
# Name "BRE_renderui - Win32 Debug"
# Name "BRE_renderui - Win32 Profile"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\source\blender\renderui\RE_renderui.h
# End Source File
# End Group
# End Target
# End Project
