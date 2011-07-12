// cDemoEffect.h: interface for the CDemoEffect class.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __CDEMOEFFECT_H_
#define __CDEMOEFFECT_H_


#include "CEnvInfo.h"	// Environment info object
#include	<iostream>		// ???TBH - debugging

class CDemoEffect  
{
	///////////////////////////////////////////////////////////////////////////
	//	METHODS
	///////////////////////////////////////////////////////////////////////////
public:
	virtual ~CDemoEffect();

	virtual bool init()				= 0;	// Initialize effect
	virtual bool unInit()			= 0;	// un-initialize effect
   virtual bool start()          = 0;  // Effect is starting, called before first advanceFrame()
   virtual bool stop()           = 0;  // Effect is stopping, called after last advanceFrame()
	virtual bool advanceFrame()	= 0;	// calculate the frame
	virtual bool renderFrame()		= 0;	// actually drawing a frame

   inline bool getStarted()               { return m_bStarted; }
   inline void setStarted(bool bStarted)  { m_bStarted = bStarted; }

private:
   bool  m_bStarted;                   // True if this class has been initialized

protected:
	CDemoEffect(); 							// construct default

};


#endif // __CDEMOEFFECT_H_