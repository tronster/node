//////////////////////////////////////////////////////////////////////////////
//
//	cParticle.h
//	Todd "Tronster" Hartley
//
//	2000.07.25
//	-	Defines a particle for a particle engine
//	-	geoGL by Moby Disk (http://www.mobydisk.com)
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __cParticle_h__
#define __cParticle_h__

#include "geometry3d.h"		// Bring in Moby Disk's geometry library


class CParticle
{
// ---------- MEMBERS ----------
public:
	// Position	
	geoGL::fVector3D	pos;				// position of a particle
	geoGL::fVector3D	dir;				// direction of movement (velocity)
	
	// Color related
	geoGL::fRGBA		color;			// color of particle
	geoGL::fRGBA		colorChange;	// direction of color transition

	// Other 
	int					life;				// life of the particle (0 = dead)
	int					status;			// status of particle (user defined)
	int					texture;			// associated gl texture number

// ---------- METHODS ----------
public:
	
	CParticle(float x=0.0f, float y=0.0f, float z=0.0f);
	CParticle(const CParticle &copy);

	// Resets a particle	
	bool reset( CParticle &master,
					CParticle &variance,
					bool bGiveLife = true);
};


#endif // __cParticle_h__