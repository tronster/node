//////////////////////////////////////////////////////////////////////////////
//
//	cParticleEngine.h
//	Todd "Tronster" Hartley
//
//	2000.07.25
//	-	Defines a particle engine
//	-	geoGL by Moby Disk (http://www.mobydisk.com)
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __cParticleEngine_h__
#define __cParticleEngine_h__

#include <vector>				// STL collection
#include "geometry3d.h"		// Bring in Moby Disk's geometry library
#include "CParticle.h"		// defines the particles used within the engine


class CParticleEngine
{
// ---------- MEMBERS ----------
public:
	// Used to access special particles inside the engine
	enum {eMASTER = -2 , eVARIANCE = -1} eSPECIAL_PARTICLES;

	// Used to turn on/off certain behaviors of the engine
	enum
	{
		eSIMPLE	= 0x0000,
		eGRAVITY	= 0x0001,
		eFADE		= 0x0002,
		eSCALE	= 0x0004
	} eENGINE_OPTIONS;

	CParticle	particleMaster;	//	Initial particle values
	CParticle	particleVariance;	// Initial particle variance (velocites)
	std::vector<CParticle>	p;		// collection of particles
	int m_iParticles;					// number of particles in system
	geoGL::fVector3D			grav;	// gravity vector
	geoGL::fVector3D			scale;// scale vector

protected:
	int	engineState;				// what options are enabled for the engine
	bool	bGravity;					// Gravity active?
	bool	bFade;						// Fade active?	???TBH - implement
	bool	bScale;						// Scale active?	???TBH - implement
	bool	m_bLifeJitter;				// The next pass, should life be randomized?

// ---------- METHODS ----------
public:
	
	// Creates a particle
	CParticleEngine(int iParticles = 1, int iEngineState = eSIMPLE);
	~CParticleEngine();

	// Turn on or off features of the particle engine
	bool setState(bool bEnable, int iNewState);

	// Initialize the particles.
	bool init(int iParticles, bool bLifeJitter = false);

	// Set a particle to a specific set of values
	bool setParticle(	int iParticle,
							float x, float y, float z,
							float dx, float dy, float dz,
							float r,	float g,	float b,
							float dr, float dg, float db,
							float life, int status, int texture);	
	
	// Reset the entire particle system
	bool resetAll();

	// Move forward one unit in time
	bool advance(float fAdjust = 1.0f);
};


#endif // __cParticleEngine_h__