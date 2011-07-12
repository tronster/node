//////////////////////////////////////////////////////////////////////////////
//
//	cVidGLDerive.h
//	2000.07.23 - Todd "Tronster" Hartley / MiNDWaRE
//
//	-	VidGL for OpenGL screen handling
//	-	Mediaduke used for texture loading
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __CVidGLDerive_h_
#define __CVidGLDerive_h_


#include "CEnvInfo.h"			// Environment info class
#include "vidgl.h"				// OpenGL environment
#include <fstream>            // For disk writing

// Effects go here
#include "CFractalIntro.h"    // Background, fractal tree, guy talking...
#include "CComaLogo.h"			// Coma2 made of "nodes" effect
#include "CSparkles.h"			// Sparkle thingy's moving across screen.
#include "CCubeZoom.h"			// Cubes rotating around cubes...
#include "CMotherboard.h"     // motherboard fly-through
#include "CShatter.h"         // shatter screen effect
#include "CCubesACM.h"        // Cubes moving into background

	///////////////////////////////////////////////////////////////////////////
	// TGA file used for writing frames to disk
	///////////////////////////////////////////////////////////////////////////
#pragma pack(push)
#pragma pack(1)

typedef unsigned char BYTE;
typedef struct
{
  BYTE           identsize;         // bytes seperating header+data
  BYTE           colormaptype;      // type of color map(0)
  BYTE           imgtype;           // (1=uncompressed pal, 2=unc RGB, 3=unc mono, other RLE)
  unsigned short colormapstart;
  unsigned short colormaplength;
  BYTE           colormapbits;
  unsigned short x_start,y_start;
  unsigned short width,depth;       // dimensions
  BYTE           bits;              // bits of color
  BYTE           descriptor;        // upside down/backwords bits
} TGAheader;
#pragma pack(pop)


class CVidGLDerive : public vidGL
{
	///////////////////////////////////////////////////////////////////////////
	// MEMBERS
	///////////////////////////////////////////////////////////////////////////
private:
	CEnvInfo			oEnvInfo;	      // environment info (width, height, etc...)
   unsigned int   unTimeSkip;       // Time in ms to skip ahead in demo at startup

	bool				m_bLoading;				// Whether we are loading or not.
	unsigned int 	m_nLoadingTexture;	// Texture for loading screen.

	// Demo effect classes
	CFractalIntro  *m_pFractalIntro; // Background, fractal tree, guy talking...
	CSparkles		*m_pSparkles;		// Sparkle thingy's moving across screen.
	CCubeZoom		*m_pCubeZoom;		// Cubes rotating around cubes...
	CComaLogo      *m_pComaLogo;		// Displays "COMA2" made out of "nodes"
   CMotherboard   *m_pMotherboard;  // Motherboard fly-through
   CCubesACM      *m_pCubesACM;     // Cubes moving into background
   CShatter       *m_pShatter1,     // Shatter screen effect #1 -- after COMA2 logo
                  *m_pShatter2;     // Shatter screen effect #2 -- "unshatter" the motherboard


   // This is used for writing to disk
   std::ofstream     m_oOutput;     // For outputting TGA files
   TGAheader         m_oTGAHeader;  // TGA header
   refptr<BYTE>      m_pImageRender;// Holds the last frame rendered
   geoGL::TextureID  m_textRender;  // Holds the texture for m_pImageRender

	///////////////////////////////////////////////////////////////////////////
	// METHODS 
	///////////////////////////////////////////////////////////////////////////
private:	
	bool	cleanup();
	void	reSizeGLScene(GLsizei width, GLsizei height);

public:

	CVidGLDerive();
	~CVidGLDerive();
	
	bool	init();  

   // Set envinfo via command line
	void	parseCommandLine(int argc, char **argv);

   // Set envinfo via parameters (for dialog box support)
   void  setCommandLineOptions(int nWidth, int nHeight, int nDepth, 
                               int nQuality, bool bFullscreen, bool bPlayMusic,
                               bool bLoop, int nSaveToDisk);

   // Get envinfo
   inline const CEnvInfo &getEnvInfo() const { return oEnvInfo; }

	bool	loadDemoMedia();
   bool  effectAdvance(unsigned int start, unsigned int stop, CDemoEffect *pEffect);
   bool  effectRender(unsigned int start, unsigned int stop, CDemoEffect *pEffect);

	// vidGL virtual overrides
	bool	renderFrame();
	bool	keyEvent(int nKey, bool bPress);
	bool	mouseEvent(float x,float y) { return true; };
	bool	advanceFrame();

};


#endif // __CVidGLDerive_h_
