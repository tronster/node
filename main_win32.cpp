//////////////////////////////////////////////////////////////////////////////
//
//	main_win32.cpp
//	2000.08.10
//
//	NODE Demo - by MiNDWaRe
//	(Bill "Moby Disk" Garrison & Todd "Tronster" Hartley)
//
//	- This module encapsulates OS specific functionality to start/end the demo
// - WIN32 version
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include "cVidGLDerive.h"
#include "resource.h"

// WIN32 dialog procedure for configuration dialog
LONG CALLBACK ConfigDlg(HWND,UINT,WPARAM,LPARAM);

//////////////////////////////////////////////////////////////////////////////
//
//	Entry point into the whole freaking routine.  *yippy*
//
//	ARGS:	...
//			...
//			lpCommandLine,		any arguements passed on command line
//			...
//
//////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCommandLine, int)
{  
	try {
   	CVidGLDerive oVidGLImpl;
		oVidGLImpl.parseCommandLine(__argc,__argv);

      // No command line args?
      //   - Show a dialog box
      //   - The dialog box will set the options
      if (__argc<=1)
      {
         InitCommonControls();
         int nResult = 
            DialogBoxParam(
               GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_CONFIG),
               NULL,(DLGPROC)ConfigDlg,(LPARAM)&oVidGLImpl);

         // User clicked cancel, exit
         if (nResult != IDOK)
            return 0;
      }

      // Initialize demo (loading, window setup, etc.)
		oVidGLImpl.init();

      // Now run the demo
		oVidGLImpl.runGL();
      return 0;
   }
	catch (char *errMsg)
	{
      std::cout << "NODE ERROR: " << errMsg << std::endl;
      MessageBox(NULL, errMsg, "MiNDWaRe - Node", MB_OK | MB_ICONERROR);
      return 1;
   }
   catch (const std::exception &e)
   {
      std::cout << "NODE ERROR: " << e.what() << std::endl;
      MessageBox(NULL, e.what(), "MiNDWaRe - Node", MB_OK | MB_ICONERROR);
      return 1;
   }
}

LONG CALLBACK ConfigDlg(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
   // HWNDs to controls
   static HWND hQualitySlider;
   static HWND hQualityMsg;
   static HWND hResBox;
   static HWND hBitBox;

   // Bit depths
   static LPCTSTR szBitMsg[] = { "16 bit","32 bit" };
   static int nBit[] = { 16,32 };
   static const int numBits = sizeof(szBitMsg)/sizeof(LPCTSTR);

   // Resolutions
   static LPCTSTR szResolutionMsg[] = {
      "256x192","512x384","320x240", "640x480", "800x600", "1024x768", "1152x864",
      "1280x1024", "1600x1200", "2048x1536" };
   static int nWidth[]  = {256,512,320,640,800,1024,1152,1280,1600,2048};
   static int nHeight[] = {192,384,240,480,600,768,864,1024,1200,1536};
   static const int numResolutions = sizeof(szResolutionMsg)/sizeof(LPCTSTR);

   static int nResMap[numResolutions];

   // Envinfo for demo
   static CVidGLDerive *oVidGLImpl = NULL;

   // Quality descriptions
   static LPCTSTR szQualityMsg[] = {
      "0 - Bad Quality\n   No textures or lighting",
      "1 - Poor Quality\n   Limited video acceleration",
      "2 - Low- Quality\n  350MHz, TNT",
      "3 - Low Quality\n  350MHz, TNT2",
      "4 - Medium- Quality\n   500MHz, TNT2",
      "5 - Medium Quality\n   500MHz, GeForce 256",
      "6 - Medium+ Quality\n   600MHz, GeForce 256",
      "7 - Good Quality\n   650MHz, GeForce DDR",
      "8 - Good+ Quality\n   800MHz, GeForce 2",
      "9 - Excellent Quality\n   High-end processor and video",
      "10 - Awesome Quality\n   Unimaginable power" };

   switch(msg)
   {
      case WM_INITDIALOG:
      {
         // Grab envinfo structure
         oVidGLImpl = (CVidGLDerive *)lParam;
         const CEnvInfo &oEnvInfo = oVidGLImpl->getEnvInfo();
         for (int nCurRes=0; nCurRes<numResolutions; nCurRes++)
            if (nWidth[nCurRes] == oEnvInfo.nWinWidth) break;
         for (int nCurBit=0; nCurBit<numBits; nCurBit++)
            if (nBit[nCurBit] == oEnvInfo.nBitsPerPixel) break;

         // Show window, set icon
         ShowWindow(hwnd,SW_SHOWNORMAL);
         //SetClassLong(hwnd,GCL_HICON,(LONG)appIcon);     // Set the icon for the dialog box

         // Setup quality slider: range 0 - 10, default of 5
         hQualitySlider = GetDlgItem(hwnd,IDC_QUALITY);
         hQualityMsg    = GetDlgItem(hwnd,IDC_QUALITYMSG);
         SendMessage(hQualitySlider,TBM_SETRANGE,FALSE,MAKELONG(0,10));
         SendMessage(hQualitySlider,TBM_SETPAGESIZE,0,1);
         SendMessage(hQualitySlider,TBM_SETPOS,TRUE,oEnvInfo.nDemoQuality);
         PostMessage(hwnd,WM_HSCROLL,0,(LPARAM)hQualitySlider);

         // Setup resolution and bit depth drop downs
         hResBox = GetDlgItem(hwnd,IDC_RESOLUTION);
         hBitBox = GetDlgItem(hwnd,IDC_BITDEPTH);
         for (int i=0; i<numResolutions; i++)
         {
            int nListItem = SendMessage(hResBox,CB_ADDSTRING,0,(LPARAM)szResolutionMsg[i]);
            // Find which list item maps to which resolution
            nResMap[nListItem] = i;
         }
         for (int j=0; j<numBits; j++)
            SendMessage(hBitBox,CB_ADDSTRING,0,(LPARAM)szBitMsg[j]);
         SendMessage(hResBox,CB_SETCURSEL,nCurRes,0);
         SendMessage(hBitBox,CB_SETCURSEL,nCurBit,0);

         // Loop and full screen checkboxes
         CheckDlgButton(hwnd,IDC_FULLSCREEN,oEnvInfo.bFullScreen);
         CheckDlgButton(hwnd,IDC_PLAYMUSIC,oEnvInfo.bPlayMusic);
         CheckDlgButton(hwnd,IDC_LOOP,oEnvInfo.bLoopForever);

         return TRUE;
      }

      // Update quality message
      case WM_HSCROLL:
         if ((HWND)lParam==hQualitySlider)
         {
            DWORD dwPos = SendMessage(hQualitySlider, TBM_GETPOS, 0, 0); 
            SendMessage(hQualityMsg,WM_SETTEXT,0,(LPARAM)szQualityMsg[dwPos]);
            return TRUE;
         }
         return FALSE;
  
      // Button press - BEGIN or EXIT
      case WM_COMMAND:
      {
         switch(LOWORD(wParam))
         {
            // Special limitations when they save frames
            case IDC_SAVEFRAMES:
            {
               // Always be at full quality, 32-bit
               SendMessage(hQualitySlider, TBM_SETPOS, TRUE, 10);
               SendMessage(hQualityMsg,WM_SETTEXT,0,(LPARAM)"Always use high quality for rendering");
               SendMessage(hBitBox,CB_SETCURSEL,1,0);

               // Default to 256x192 resolution
               SendMessage(hResBox,CB_SETCURSEL,0,0);

               // Shut off music, full-screen, and looping
               CheckDlgButton(hwnd,IDC_FULLSCREEN,FALSE);
               CheckDlgButton(hwnd,IDC_PLAYMUSIC,FALSE);
               CheckDlgButton(hwnd,IDC_LOOP,FALSE);

               // Disable dialog boxes if save frames is checked
               bool bEnable = IsDlgButtonChecked(hwnd,IDC_SAVEFRAMES) == FALSE;
               EnableWindow(hQualitySlider, bEnable);
               EnableWindow(hBitBox, bEnable);
               EnableWindow(hBitBox, bEnable);
               EnableWindow(GetDlgItem(hwnd,IDC_QUALITYSTATIC), bEnable);
               EnableWindow(GetDlgItem(hwnd,IDC_FULLSCREEN), bEnable);
               EnableWindow(GetDlgItem(hwnd,IDC_PLAYMUSIC), bEnable);
               EnableWindow(GetDlgItem(hwnd,IDC_LOOP), bEnable);

               SendMessage(hResBox,CB_RESETCONTENT,0,0);
               for (int i=0; i<numResolutions; i++)
                  if (bEnable || nWidth[i] == 256 || nWidth[i] == 512 ||
                                 nWidth[i] == 1024 || nWidth[i] == 2048)
                  {
                     int nListItem = SendMessage(hResBox,CB_ADDSTRING,0,(LPARAM)szResolutionMsg[i]);
                     // Find which list item maps to which resolution
                     nResMap[nListItem] = i;
                  }

               SendMessage(hResBox,CB_SETCURSEL,0,0);

               if (!bEnable)
                  ::MessageBox(hwnd,"This option is used to create a pre-rendered video of Node.\n\n"
                                    "In this special mode, Node will write out all of the frames of video "
                                    "to .TGA files at 30fps.  This option requires 1GB of free disk space "
                                    "for the 256x192 resolution. Do not use this option unless you "
                                    "have the space available to create a video file from the individual frames.",
                                    "Node by Mindware",MB_OK | MB_ICONINFORMATION);
               break;
            }

            case IDOK:
            {
               // Lookup current resolution/bit depth indices
               int nCurRes = nResMap[SendMessage(hResBox,CB_GETCURSEL,0,0)];
               int nCurBit = SendMessage(hBitBox,CB_GETCURSEL,0,0);

               // Set command line options
               oVidGLImpl->setCommandLineOptions(
                  nWidth[nCurRes],nHeight[nCurRes],nBit[nCurBit],          // Res, bit depth
                  SendMessage(hQualitySlider,TBM_GETPOS,0,0),              // Quality
                  IsDlgButtonChecked(hwnd,IDC_FULLSCREEN) ? true : false,  // Fullscreen
                  IsDlgButtonChecked(hwnd,IDC_PLAYMUSIC) ? true : false,   // Sound
                  IsDlgButtonChecked(hwnd,IDC_LOOP) ? true : false,        // Loop
                  IsDlgButtonChecked(hwnd,IDC_SAVEFRAMES) ? 1 : 0);        // Save to disk
            }
                                                   
            case IDCANCEL: EndDialog(hwnd,wParam);
         }
         return TRUE;
      }
   }

   return FALSE;
}
