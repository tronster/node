#pragma warning(disable:4786)
//////////////////////////////////////////////////////////////////////////////
//
//	cVidGLDerive.cpp
//
//	2000.08.10 - Todd Hartley - Implements VidGL for MiNDWaRe's Node demo
//
//////////////////////////////////////////////////////////////////////////////

#include "CVidGLDerive.h"


// Deletes a demo effect object, only if it is not loaded
#define SAFEDELETE(x) { if ((x) != NULL) { (x)->unInit(); delete (x); (x) = NULL; } }

// Linux uses strcasecmp instead of stricmp
#ifndef WIN32
#define stricmp(x,y)    strcasecmp(x,y)
#define strnicmp(x,y)   strncasecmp(x,y)
#endif

//////////////////////////////////////////////////////////////////////////////
//
//	TIMING FOR DEMO EFFECTS
//
//	-	Start and stop times for each effect.  These may overlap...
//
//////////////////////////////////////////////////////////////////////////////
enum {
   eDEMO_START          = 0,
   //---------------------------//
   eFRACTALINTRO_START  = 0,
   eFRACTALINTRO_STOP   = 90000,
   eCOMALOGO_START      = 90000,
   eCOMALOGO_STOP       = 135430,
   eSHATTER1_START      = 135430,
   eSHATTER1_STOP       = 145430,
	eSPARKLES_START		= 137000,
	eSPARKLES_STOP			= 149000,
	eCUBEZOOM_START		= 145000,
	eCUBEZOOM_STOP			= 193000,
   eCUBESACM_START      = 193000,
   eCUBESACM_STOP       = 223000,
	eSHATTER2_START      = 213000,
   eSHATTER2_STOP       = 223100,
	eMOTHERBOARD_START   = 223100,
   eMOTHERBOARD_STOP    = 310000,
   //---------------------------//
   eDEMO_STOP           = 311000
};



//////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
//	-	Sets up default to be an 800x600x24, non-fullscreen window
//
//////////////////////////////////////////////////////////////////////////////
CVidGLDerive::CVidGLDerive() :
	m_pFractalIntro(NULL), m_pComaLogo(NULL), m_pSparkles(NULL),
	m_pCubeZoom(NULL), m_pMotherboard(NULL), m_pCubesACM(NULL),
   m_pShatter1(NULL), m_pShatter2(NULL), m_nLoadingTexture(0),
	m_bLoading(true)

{
	oEnvInfo.nWinWidth		= 800;
	oEnvInfo.nWinHeight		= 600;
	oEnvInfo.nBitsPerPixel	= 32;
   oEnvInfo.bPlayMusic     = true;
   #ifdef _DEBUG
   oEnvInfo.bFullScreen		= false;
	oEnvInfo.bShowDebugInfo = true;
   #else
   oEnvInfo.bFullScreen		= true;
	//oEnvInfo.bShowDebugInfo = false;     // Flag does not exist in release mode
   #endif
	oEnvInfo.nDemoQuality	= 5;
	oEnvInfo.bLoopForever	= false;
   oEnvInfo.fFrameFactor   = 0;
   oEnvInfo.unStartMusicOrder = 0;
   oEnvInfo.nSaveToDisk    = 0;           // Don't save frames to disk
   unTimeSkip              = 0;           // Start at time 0
}



//////////////////////////////////////////////////////////////////////////////
//
//	DESTRUCTOR
//
//////////////////////////////////////////////////////////////////////////////
CVidGLDerive::~CVidGLDerive()
{
	cleanup();
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Sets up the demo environment
//	-	Makes vidGL call to setup the screen
//
//	PRE:	The members of oEnvInfo contain:
//				nScreenWidth,	Screen width in pixels.
//				nScreenHeight,	Screen height in pixels.
//				nScreenBits,	Bits per pixel (8, 24, 32) for the screen.
//				bFullScreen,	Whether or not full screen should be used.
//
//	RETURNS:	true, on success
//				false, on failure
//
//	THROWS:	char *,	a message on an error
//
//////////////////////////////////////////////////////////////////////////////
bool CVidGLDerive::init()
{
	vidGL::init(
		oEnvInfo.nWinWidth, 
		oEnvInfo.nWinHeight, 
		oEnvInfo.nBitsPerPixel,
		oEnvInfo.bFullScreen);

   //???WHG Hack - vidgl is generating error GL_INVALID_VALUE
   // Just ignore it
   int nIgnoreError = glGetError();

   // Set the type of frame rate control:
   if (oEnvInfo.nSaveToDisk > 0)                   // If saving to disk
      setFrameRate(30, vidGL::FPS_ALL);            // Render every frame exactly
   else
	   setFrameRate(30, vidGL::FPS_FREE);           // Otherwise remder as fast as possible

	reSizeGLScene(oEnvInfo.nWinWidth, oEnvInfo.nWinHeight);
   oEnvInfo.calcGLsettings();

   // If we are saving to disk then create a texture to hold the screen shots
   if (oEnvInfo.nSaveToDisk > 0)
   {
      // Make sure all the parameters are valid for saving to disk
      if (oEnvInfo.nWinWidth != 256 && oEnvInfo.nWinWidth != 512 && oEnvInfo.nWinWidth != 1024 && oEnvInfo.nWinWidth != 2048)
         throw "CVidGLDerive::init()\r\nWidth must be 256, 512, 1024, or 2048 for disk writing";

      if (oEnvInfo.bFullScreen)
         throw "CVidGLDerive::init()\r\nCannot use full-screen with disk writing";

      if (oEnvInfo.bPlayMusic)
         throw "CVidGLDerive::init()\r\nCannot play using when disk writing. Write audio separately.";

      if (oEnvInfo.bLoopForever)
         throw "CVidGLDerive::init()\r\nCannot loop forever when writing to disk.";

      // Allocate the generated texture image
      m_pImageRender.refnew(oEnvInfo.nWinWidth*oEnvInfo.nWinWidth*3);   // Width x Width so that it is square

      // Create non-mipmapped texture
      m_textRender.create();

	   glBindTexture(GL_TEXTURE_2D, m_textRender);
	   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      glTexImage2D(
         GL_TEXTURE_2D,       // 2D texture
         0,              // No level of detail
         3,                   // RGB
         oEnvInfo.nWinWidth,  // width must be 2^n so we use width for both
         oEnvInfo.nWinWidth,  // height must be 2^n so we use width for both
         0,                   // width of border (0 or 1)
         GL_RGB,              // what data represents (palette #s or values)
         GL_UNSIGNED_BYTE,    // type of pixel data
         m_pImageRender);     // actual image data

      // Setup TGA header - 24 bit Width x Height
      m_oTGAHeader.identsize = 0;
      m_oTGAHeader.colormaptype = 0;
      m_oTGAHeader.imgtype = 2;
      m_oTGAHeader.colormapstart = 0;
      m_oTGAHeader.colormaplength = 0;
      m_oTGAHeader.colormapbits = 0;
      m_oTGAHeader.x_start = 0;
      m_oTGAHeader.y_start = 0;
      m_oTGAHeader.width = oEnvInfo.nWinWidth;
      m_oTGAHeader.depth = oEnvInfo.nWinHeight;
      m_oTGAHeader.bits = 24;
      m_oTGAHeader.descriptor = 0;
   }

	// Open the media directory/file
	if (!oEnvInfo.oMedia.open("data.md"))
		throw "CVidGLDerive::init()\r\nUnable to open \"data.md\"";

	// Media duke image object for background.
   md::Cimage oImage;
   
	if (!oEnvInfo.oMedia.read("loading.png", oImage))
		throw "CVidGLDerive::init()\nCould not load loading.png";
	m_nLoadingTexture = oImage.makeGLTexture(oEnvInfo.glWantLinear,oEnvInfo.glWantLinear);
	oImage.destroy();

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Loads media (used throughout the whole demo.)
//
//	POST:	A "loading" picture should be on display.
//
//////////////////////////////////////////////////////////////////////////////
bool CVidGLDerive::loadDemoMedia()
{
	// Mark that we are done loading.
   std::cout << "Loading..." << std::endl;

	// Construct demo effects 
	m_pFractalIntro= new CFractalIntro(&oEnvInfo);
	m_pComaLogo    = new CComaLogo(&oEnvInfo);
	m_pSparkles		= new CSparkles(&oEnvInfo);
	m_pCubeZoom		= new CCubeZoom(&oEnvInfo);
   m_pMotherboard = new CMotherboard(&oEnvInfo);   
   m_pCubesACM    = new CCubesACM(&oEnvInfo);
   m_pShatter1    = new CShatter(&oEnvInfo);
   m_pShatter2    = new CShatter(&oEnvInfo);

	// Check for proper construction
   if (m_pFractalIntro == NULL)
      throw "CVidGLDerive::init()\r\nCould not new m_pFractalIntro";
	if (m_pComaLogo == NULL)
		throw "CVidGLDerive::init()\r\nCould not new m_pComaLogo";
	if (m_pSparkles == NULL)
		throw "CVidGLDerive::init()\r\nCould not new m_pSparkles";
	if (m_pCubeZoom == NULL)
		throw "CVidGLDerive::init()\r\nCould not new m_pCubeZoom";
   if (m_pMotherboard == NULL)
      throw "CVidGLDerive::init()\r\nCould not new m_pMotherboard";
   if (m_pCubesACM == NULL)
      throw "CVidGLDerive::init()\r\nCould not new m_pCubesACM";
	if (m_pShatter1 == NULL)
      throw "CVidGLDerive::init()\r\nCould not new m_pShatter1";
   if (m_pShatter2 == NULL)
      throw "CVidGLDerive::init()\r\nCould not new m_pShatter2";

   // Initialize demo effects
   m_pFractalIntro->init();
	m_pComaLogo->init();   
	m_pCubeZoom->init();
	m_pSparkles->init();
	m_pMotherboard->init();
   m_pCubesACM->init();
   m_pShatter1->setShatterOptions("shshot1.png",0,CShatter::eSHATTER_TUNNEL,false);
   m_pShatter1->init();
   m_pShatter2->setShatterOptions("shshot2.png",0,CShatter::eSHATTER_BACKWARD,true);
   m_pShatter2->init();

// If in debug mode, setup the virtual debug window
#ifdef _DEBUG
	if (!oEnvInfo.OglDebug.init(900,700))
		throw "CVidGLDerive::init()\r\nUnable to initialize debug library";
#endif //_DEBUG


   if (oEnvInfo.bPlayMusic)
   {
      // Initialize music player
      std::cout << "Initializing music..." << std::endl;
   	oEnvInfo.oMusic.init();

   	// Load music
      std::cout << "Loading music..." << std::endl;
   	if (!oEnvInfo.oMedia.read("perinode.it", oEnvInfo.oMusic))
      {
         throw "Unable to read perinode.it";
      }

   	// Done initialization, so let's rock
      std::cout << "Playing music..." << std::endl;
   	oEnvInfo.oMusic.play();
   	oEnvInfo.oMusic.setOrder(oEnvInfo.unStartMusicOrder);
   } else
   {
      std::cout << "Emulating music..." << std::endl;
   }

	// Mark that we are done loading.
   std::cout << "Done loading..." << std::endl;
	m_bLoading = false;

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	Cleans up the demo.
//
//	RETURNS:	true, on success
//				false, on failure
//
//////////////////////////////////////////////////////////////////////////////
bool CVidGLDerive::cleanup()
{
	// Kill our demo effect objects.
	SAFEDELETE(m_pFractalIntro);
   SAFEDELETE(m_pComaLogo);
   SAFEDELETE(m_pSparkles);
   SAFEDELETE(m_pCubeZoom);
   SAFEDELETE(m_pMotherboard);
   SAFEDELETE(m_pCubesACM);
   SAFEDELETE(m_pShatter1);
   SAFEDELETE(m_pShatter2);

	// Close the media directory/file.
	oEnvInfo.oMedia.close();

	// Kill the loading screen texture.
	if (m_nLoadingTexture != 0)
		glDeleteTextures(1, &m_nLoadingTexture );

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	Parses the command line and sets internal variables accordingly
//
//	THROWS:	char *,	a message if an error reading command line
//
//////////////////////////////////////////////////////////////////////////////
void CVidGLDerive::parseCommandLine(int argc, char **argv)
{
   std::cout   << "Node by Mindware:\n"
               << "First presented at the Coma2 demo party in Y2K.\n\n"
               << "  mwnode [-r width height bpp] [-f0 | -f1] [-q # | -quality #] [-s | -silent] [-d frameSkip]\n"
               << "\n"
               << "-r            Resolution; specify width, height, and bits-per-pixel\n"
               << "-f            Full screen disable, enable\n"
               << "-q            Quality factor 0..10...\n"
               << "-s, -silent   Disable sound\n"
               << "-d            Save frames to disk.  0 = disable, 1 = 30fps, 2 = 15fps, 3 = 10fps, ...\n"
               << std::endl;

	for (int argno=1; argno<argc; argno++)
	{
		char *option = argv[argno];
		
		if (stricmp(option, "-r")==0)
		{
			oEnvInfo.nWinWidth		= atoi(argv[argno+1]); argno++;
			oEnvInfo.nWinHeight		= atoi(argv[argno+1]); argno++;
			oEnvInfo.nBitsPerPixel	= atoi(argv[argno+1]); argno++;
		}
		else
		if (stricmp(option, "-f")==0 || stricmp(option, "-f1")==0)
			{ oEnvInfo.bFullScreen = true; }
			else
		if (stricmp(option, "-f0")==0)
			{ oEnvInfo.bFullScreen = false; }
			else
		if (stricmp(option, "-q")==0 || stricmp(option, "-quality")==0)
			{ oEnvInfo.nDemoQuality = atoi(argv[argno+1]); argno++; }
			else
      if (stricmp(option, "-s")==0 || stricmp(option, "-silent")==0)
         { oEnvInfo.bPlayMusic = false; }
         else
		if (stricmp(option, "-l")==0 || stricmp(option, "-loop")==0)
			{ oEnvInfo.bLoopForever = true; }
         else
      if (stricmp(option, "-t")==0 || stricmp(option, "-time")==0)
         { unTimeSkip = atoi(argv[argno+1]); argno++; }
         else
      if (stricmp(option, "-o")==0 || stricmp(option, "-order")==0)
         { oEnvInfo.unStartMusicOrder = atoi(argv[argno+1]); argno++;  }
         else
      if (stricmp(option, "-d")==0 || stricmp(option, "-disk")==0)
         { oEnvInfo.nSaveToDisk = atoi(argv[argno+1]); argno++;  }
  }
}

//////////////////////////////////////////////////////////////////////////////
//
//	Sets the internal variables directly from parameters
//   - This is used for configuration via other means (dialog box, .ini file)
//
//	THROWS:	char *,	a message if an error reading command line
//
//////////////////////////////////////////////////////////////////////////////
void CVidGLDerive::setCommandLineOptions(int nWinWidth, int nWinHeight, int nBitsPerPixel, 
                           int nDemoQuality, bool bFullScreen, bool bPlayMusic, bool bLoopForever,
                           int nSaveToDisk)
{
   // Just copy values into envinfo structure
   oEnvInfo.nWinWidth      = nWinWidth;
   oEnvInfo.nWinHeight     = nWinHeight;
   oEnvInfo.nBitsPerPixel  = nBitsPerPixel;
   oEnvInfo.nDemoQuality   = nDemoQuality;
   oEnvInfo.bFullScreen    = bFullScreen;
   oEnvInfo.bPlayMusic     = bPlayMusic;
   oEnvInfo.bLoopForever   = bLoopForever;
   oEnvInfo.nSaveToDisk    = nSaveToDisk;
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Advance a demo effect if it is time
//	-	Calls start(), stop() on the effect as needed
//
//	ARGS:	start,			Start time in milliseconds for effect.
//			stop,				End time in milliseconds for effect.
//			pThisEffect.	Pointer to effect object.
//
//	PRE:	Call from advanceFrame()
//
//	RETURNS:	true if demo effect is on screen
//
//////////////////////////////////////////////////////////////////////////////
bool CVidGLDerive::effectAdvance(	unsigned int start, unsigned int stop,
												CDemoEffect *pThisEffect)
{
   unsigned int   nCurTime = oEnvInfo.getMusicTime();	// Current Time
   bool           bRet     = false;							// if effect rendered

   // Effect is currently running:
   if (nCurTime>=start && nCurTime<stop)
   {
      // First time effect is run:
      if (!pThisEffect->getStarted())
      {
         pThisEffect->start();								// Start effect
         pThisEffect->setStarted(true);					// Set started flag
      }
      pThisEffect->advanceFrame();							// Advance effect
      bRet = true;
   }

   // Effect is not running:
   else
   {
      // Effect is started:
      if (pThisEffect->getStarted())
      {
         pThisEffect->stop();									// Stop effect
         pThisEffect->setStarted(false);					// Clear started flag
      }
         
      bRet = false;
   }

   return bRet;
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Render a demo effect if it is time
//
//	ARGS:	start,			Start time in milliseconds for effect.
//			stop,				End time in milliseconds for effect.
//			pThisEffect.	Pointer to effect object.
//
//	PRE:	Call from renderFrame()
//
//	RETURNS:	true if demo effect is on screen
//
//////////////////////////////////////////////////////////////////////////////
bool CVidGLDerive::effectRender(	unsigned int start, unsigned int stop,
											CDemoEffect *pThisEffect)
{
   unsigned int   nCurTime = oEnvInfo.getMusicTime();	// Current Time
   bool           bRet     = false;							// if effect rendered

   if (nCurTime >= start  &&  nCurTime < stop)			// Time to run effect?
   {
      pThisEffect->renderFrame();							// Render effect
      bRet = true;
   }

   return bRet;
}



//////////////////////////////////////////////////////////////////////////////
//
// -	Slick macros for calling effectRender(), effectAdvance()
// -	This is not good programming style, but it looks neat.  I'll never get
//		to use this macro trick again without getting in trouble. -WHG
//
//////////////////////////////////////////////////////////////////////////////
#define effectAdvance(EFFECT,object)  \
	effectAdvance(e##EFFECT##_START,e##EFFECT##_STOP,object)

#define effectRender(EFFECT,object)	\
	effectRender(e##EFFECT##_START,e##EFFECT##_STOP,object)



//////////////////////////////////////////////////////////////////////////////
//
//	-	Does the math which detmines the scene next time it's rendered.
//
//	RETURNS:	true, Always whether you like it or not.
//
//////////////////////////////////////////////////////////////////////////////
bool CVidGLDerive::advanceFrame()
{
	// Don't enter if loading start graph.
	if (m_bLoading)
		return true;

   // Update frame info
   oEnvInfo.fFrameFactor = getFrameFactor();

   // Get time from music - if not, just use the frame timers in vidGL
   if (oEnvInfo.bPlayMusic)
      oEnvInfo.unMusicTime = oEnvInfo.oMusic.getTime() + unTimeSkip;
   else
      oEnvInfo.unMusicTime = static_cast<int>(getFrameCount() * getFrameTimeSet());

   // Advance the appropriate effect (using Moby Disk's slick macro wrapper)
   effectAdvance(FRACTALINTRO,m_pFractalIntro);
   effectAdvance(COMALOGO,    m_pComaLogo);
   effectAdvance(SPARKLES,		m_pSparkles);
   effectAdvance(CUBEZOOM,		m_pCubeZoom);
   effectAdvance(MOTHERBOARD, m_pMotherboard);
   effectAdvance(CUBESACM,    m_pCubesACM);
   effectAdvance(SHATTER1,    m_pShatter1);
   effectAdvance(SHATTER2,    m_pShatter2);

   // Check for completion
   if (oEnvInfo.unMusicTime > eDEMO_STOP)
      endGL();

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	- Actually draws the scene
//
//	RETURNS:	true, Always whether you like it or not.
//
//////////////////////////////////////////////////////////////////////////////
bool CVidGLDerive::renderFrame()
{
	// Clear the screen and depth buffers.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Only perform effects if not loading
	if (!m_bLoading)
	{	
		// Render the appropriate effect (using another Moby Disk slick macro wrapper)
		effectRender(FRACTALINTRO, m_pFractalIntro);
		effectRender(COMALOGO,     m_pComaLogo);
		effectRender(SPARKLES,		m_pSparkles);
		effectRender(CUBEZOOM,		m_pCubeZoom);
		effectRender(MOTHERBOARD,  m_pMotherboard);
		effectRender(CUBESACM,     m_pCubesACM);
		effectRender(SHATTER1,     m_pShatter1);
		effectRender(SHATTER2,     m_pShatter2);
   
      // Write a frame to disk
      if (oEnvInfo.nSaveToDisk > 0)
      {
         // Grab the screen onto this texture
         glBindTexture(GL_TEXTURE_2D, m_textRender);
         glCopyTexSubImage2D(GL_TEXTURE_2D,0, 0,0,
                             0,0,
                             oEnvInfo.nWinWidth,oEnvInfo.nWinHeight);

         // Copy the texture into memory
         glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, m_pImageRender);

         // Write the texture to disk
         // If nSaveToDisk > 0 then this will reduce the frame rate
         if (getFrameCount() % oEnvInfo.nSaveToDisk == 0)
         {
            char sFileName[25];
            sprintf(sFileName,"node_%08u.tga",getFrameCount());
            m_oOutput.open(sFileName,std::ios::out | std::ios::binary | std::ios::trunc);
            m_oOutput.write(reinterpret_cast<char *>(&m_oTGAHeader),sizeof(m_oTGAHeader));
            m_oOutput.write(reinterpret_cast<const char *>(static_cast<const unsigned char *>(m_pImageRender)),oEnvInfo.nWinWidth*oEnvInfo.nWinHeight*3);
            m_oOutput.close();
         }
      }

	} else {

		static bool s_bFirstTime = true;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
		glDisable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);

		glBindTexture(GL_TEXTURE_2D, m_nLoadingTexture);
		glBegin(GL_QUADS);	
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(-2.0f, -2.0f, -3.6f);			
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(2.0f, -2.0f, -3.6f);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(2.0f, 2.0f, -3.6f);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(-2.0f, 2.0f, -3.6f);
		glEnd();	

		// Wait until second pass so screen will display.
		if (!s_bFirstTime)
			loadDemoMedia();

		s_bFirstTime = false;

		return true;
	}

	// Common debug messages to show during all routines
#ifdef _DEBUG
	if (oEnvInfo.bShowDebugInfo)
	{
      // White no matter what other opengl stuff is happening
      float _white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT,GL_EMISSION,_white);

		// Frames-per-second
		oEnvInfo.OglDebug.printf(0, 0, 0, "FPS: %d", getFrameRateSmooth() );

		// Number of frames drawn out of total frame calls
		oEnvInfo.OglDebug.printf(0, 16, 0, "Frames: %d / %d",
			getFrameDrawCount(), getFrameCount() );
		
		// Music information
		oEnvInfo.OglDebug.printf(0, 32, 0, "Time: %d,  Music: %d.%d",
			oEnvInfo.getMusicTime(),
			oEnvInfo.getMusicRow(),
			oEnvInfo.getMusicOrder());

      // White no matter what other opengl stuff is happening
      float _black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glMaterialfv(GL_FRONT,GL_EMISSION,_black);
	}
#endif //_DEBUG	

	glFlush();

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	Handles a key being pressed
//
//	ARGS:	nKey,	a value representing a key on the keyboard
//			bPress, [true] is true if a key is being pressed, false if released
//
//	RETURN:	true, on success
//				false, otherwise
//
//////////////////////////////////////////////////////////////////////////////
bool CVidGLDerive::keyEvent(int nKey, bool bPress)
{
	// Exit if key is released
   if (!bPress)
      return true;

   switch(nKey)
	{
      // Quit if ESC is hit
		case VGL_ESCAPE:
         endGL();
         break;

#ifdef _DEBUG
		// Toggle debug info on/off
		case 'D':
         oEnvInfo.bShowDebugInfo = !oEnvInfo.bShowDebugInfo;
			break;
#endif // _DEBUG

	}

   return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Resizes the Win32 window, as well as recompute's the perspective
//	-	If a routine uses it's own perspective or plane settings, which aren't
//		called every frame, then things are going to look ummmm 'funky' until
//		the next effect starts.
//
//	ARGS:	width,	Window's width.
//			height,	Window's height.
//
//////////////////////////////////////////////////////////////////////////////
void CVidGLDerive::reSizeGLScene(GLsizei width, GLsizei height)
{
	// Prevent a divide by zero
	if (height == 0)
		height=1;

	// Set lower-left corner to 0,0 & dimensions to the Win32 window
	glViewport(0, 0, width, height);

	// Reset projection matrix
	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();

	// Calculate The Aspect Ratio Of The Window (default, effects may change this)
	gluPerspective(
		45.0f,									// field of view
      4.0f/3.0f,                       // force 4/3 aspect ratio even if the window is stretched
		2.0f,										// "near" plane to camera
		260.0f);									// "far" plane to camera

	// Reset modle view
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();							
}
