//////////////////////////////////////////////////////////////////////////////
//
// CEnvInfo.h
//	
//	Class to hold environment information
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __CENVINFO_H_
#define __CENVINFO_H_

// Debug routines
#ifdef _DEBUG
#include "glDebug.h"
#endif // _DEBUG

#include "mduke.h"				// Media Duke - gfx/music loader


class CEnvInfo  
{
	///////////////////////////////////////////////////////////////////////////
	// MEMBERS
	///////////////////////////////////////////////////////////////////////////
public:
	// Quality settings
	int	nWinWidth;			// Width of screen/window in pixels.
	int	nWinHeight;			// Height of screen/window in pixels.
	int	nBitsPerPixel;		// Bits per screen pixel.
	int	nDemoQuality;		// How much quality (lines/tris/etc...)
   int   glWantMipmap,     // Filtering to use if mipmapping is requested
         glWantLinear;     // Filtering to use if linear is requested
	
	bool	bFullScreen;		// Are we running in full screen mode?	
	bool	bLoopForever;		// Keep going after it ends?
   int   nSaveToDisk;      // Divisor for save to disk (0 = no save, 1 = all frames, 2 = 1/2 frames)
   float fFrameFactor;     // Value of vidGL::getFrameFactor() for speed/timing
	
	// Music related
   bool  bPlayMusic;       // true to play music, false for silence
	unsigned int unStartMusicOrder;	// Order to start music (for debugging)
   unsigned int unMusicTime;			// Current time in the music
	
	md::CmediaDuke oMedia;	// Class for loading graphics & sound.
	md::Cmusic		oMusic;	// Music object.

#ifdef _DEBUG
	CglDebug OglDebug;      // debug class for debugging output
	bool bShowDebugInfo;    // Flag to indicate visibility of debug info
#endif // _DEBUG

		
	///////////////////////////////////////////////////////////////////////////
	// METHODS 
	///////////////////////////////////////////////////////////////////////////
public:
	CEnvInfo();
	virtual ~CEnvInfo();

   // Calculate the values for glWantMipmap and glWantLinear from the nDemoQuality
   void calcGLsettings();

	// Accessors
	// Time since music started.
	inline unsigned int	getMusicTime() { return unMusicTime; }
	// Mod's current order.
	inline int				getMusicOrder()	{return oMusic.getOrder();}
	// Mod's current position in a pattern.
	inline int				getMusicRow()		{return oMusic.getRow();}
	
};

#endif // __CENVINFO_H_
