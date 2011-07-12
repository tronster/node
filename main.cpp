//////////////////////////////////////////////////////////////////////////////
//
//	main.cpp
//	2001.02.05
//
//	NODE Demo - by MiNDWaRe
//	(Bill "Moby Disk" Garrison & Todd "Tronster" Hartley)
//
//	- This module encapsulates OS specific functionality to start/end the demo
// - Linux version
//////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "CVidGLDerive.h"

//////////////////////////////////////////////////////////////////////////////
//
//	Entry point into the whole freaking routine.  *yippy*
//
//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{  
	try {
   	CVidGLDerive oVidGLImpl;
		oVidGLImpl.parseCommandLine(argc,argv);

      // ???WHG Dialog box goze here
      // ???WHG Maybe at least print command line args
      // ???WHG opengl dialog box that is cross platform?

      // Initialize demo (loading, window setup, etc.)
		oVidGLImpl.init();

      // Now run the demo
		oVidGLImpl.runGL();
      return 0;
   }
	catch (const char *errMsg)
	{
      std::cout << "NODE ERROR: " << errMsg << std::endl;
      //???WHG Message box
      return 1;
   }
   catch (const std::exception &e)
   {
      std::cout << "NODE EXCEPTION: " << e.what() << std::endl;
      //???WHG Message box
      return 1;
   }
   catch (...)
   {
      std::cout << "Unknown exception" << std::endl;
      return 2;
   }
}
