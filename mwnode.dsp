# Microsoft Developer Studio Project File - Name="mwnode" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=mwnode - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mwnode.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mwnode.mak" CFG="mwnode - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mwnode - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "mwnode - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/mwNode", FCAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mwnode - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\vidgl_win32" /I "..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 geogl.lib gldebug.lib mduke.lib fmodvc.lib glpng.lib vidgl.lib glaux.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /machine:I386 /out:"mwnode.exe" /libpath:"..\vidgl_win32" /libpath:"..\common\lib"

!ELSEIF  "$(CFG)" == "mwnode - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /Zi /Od /Gy /I "..\vidgl_win32" /I "..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fr /FD /GZ /c
# SUBTRACT CPP /Gf /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 geogld.lib mduked.lib fmodvc.lib gldebugd.lib glpngd.lib vidgld.lib glaux.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"mwnode.exe" /pdbtype:sept /libpath:"..\vidgl_win32" /libpath:"..\common\lib"
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "mwnode - Win32 Release"
# Name "mwnode - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CComaLogo.cpp
# End Source File
# Begin Source File

SOURCE=.\CCubesACM.cpp
# End Source File
# Begin Source File

SOURCE=.\CCubeZoom.cpp
# End Source File
# Begin Source File

SOURCE=.\cDemoEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\CEnvInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\CFractalIntro.cpp
# End Source File
# Begin Source File

SOURCE=.\CIFS.cpp
# End Source File
# Begin Source File

SOURCE=.\CMotherboard.cpp
# End Source File
# Begin Source File

SOURCE=.\cParticle.cpp
# End Source File
# Begin Source File

SOURCE=.\cParticleEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\CShatter.cpp
# End Source File
# Begin Source File

SOURCE=.\CSparkles.cpp
# End Source File
# Begin Source File

SOURCE=.\cVidGLDerive.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp

!IF  "$(CFG)" == "mwnode - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "mwnode - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\main_win32.cpp
# End Source File
# Begin Source File

SOURCE=.\mwnode.rc
# End Source File
# Begin Source File

SOURCE=.\tiler.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CComaLogo.h
# End Source File
# Begin Source File

SOURCE=.\CCubesACM.h
# End Source File
# Begin Source File

SOURCE=.\CCubeZoom.h
# End Source File
# Begin Source File

SOURCE=.\cDemoEffect.h
# End Source File
# Begin Source File

SOURCE=.\CEnvInfo.h
# End Source File
# Begin Source File

SOURCE=.\CFractalIntro.h
# End Source File
# Begin Source File

SOURCE=.\CIFS.h
# End Source File
# Begin Source File

SOURCE=.\CMotherboard.h
# End Source File
# Begin Source File

SOURCE=.\cParticle.h
# End Source File
# Begin Source File

SOURCE=.\cParticleEngine.h
# End Source File
# Begin Source File

SOURCE=.\CShatter.h
# End Source File
# Begin Source File

SOURCE=.\CSparkles.h
# End Source File
# Begin Source File

SOURCE=.\cVidGLDerive.h
# End Source File
# Begin Source File

SOURCE=.\tiler.h
# End Source File
# Begin Source File

SOURCE=..\Common\h\vidGL.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
