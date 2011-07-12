//////////////////////////////////////////////////////////////////////////////
//
//	cParticleEngine.cpp
//	Todd "Tronster" Hartley
//
// 2000.07.30
//	-	Basic engine in place, need to add state machine for enable commands:
//		1) scale particle down as it dies
//		2) scale factor
//		3) yes/no one particle re-created per cycle
//
//	2000.07.25
//	-	Implements a particle engine
//
//////////////////////////////////////////////////////////////////////////////

#include "CDemoEffect.h"
#include "CParticleEngine.h"

//////////////////////////////////////////////////////////////////////////////
//
//	- After constructing, the particle engine is ready to go.
//
//	ARGS:	[in] iParticles,	[0] the number of particles to create
//
//////////////////////////////////////////////////////////////////////////////
CParticleEngine::CParticleEngine(int iParticles, int iEngineState) :
	bGravity(false),
	bFade(false),
	bScale(false)
{
	setState(true, iEngineState);
	init(iParticles);
}



CParticleEngine::~CParticleEngine()
{
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Turn on or off features of the particle engine
//	-	If the new state is eSIMPLE, it resets the engine state
//
//	ARGS:	bEnable,		if true then the new state option(s) is turned on, if
//							false it is turned off
//			iNewState,	the state(s) to turn on/off
//
//	RETURN:	true, on success
//				false, otherwise
//////////////////////////////////////////////////////////////////////////////
bool CParticleEngine::setState(bool bEnable, int iNewState)
{
	// Simplest case is a reset
	if (iNewState == eSIMPLE)
	{
		bGravity = false;
	}

	if (iNewState & eGRAVITY)
		bGravity = bEnable;

	if (iNewState & eFADE)
		bFade = bEnable;

	if (iNewState & eSCALE)
		bScale = bEnable;

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	Resets entire engine
//
//	ARGS:	iParticles,		Number of particles to initialize.
//			bLifeJitter,	[false] Should the life of particles be randomized.
//
//////////////////////////////////////////////////////////////////////////////
bool CParticleEngine::init(int iParticles, bool bLifeJitter)
{
	if (iParticles < 1)
		iParticles = 1;

	// Next update, particles life will be randomized if true.
	m_bLifeJitter = bLifeJitter;

	p.resize(iParticles);		// attempt to allocate a number of particles

	m_iParticles = p.size();	// set the # of particles allocated internally
	
	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Sets a particle to certain values
//
//	ARGS:	iParticle,	the particle number or ENUM of the particle to set
//
//	RETURN:	true, on success
//				false, otherwise
//////////////////////////////////////////////////////////////////////////////
bool CParticleEngine::setParticle(	int iParticle,
												float x, float y, float z,
												float dx, float dy, float dz,
												float r,	float g,	float b,
												float dr, float dg, float db,
												float life, int status, int texture)
{

	// Determine if we are using a "special" particle or just setting one
	// of the particles in our vector collection
	switch (iParticle)
	{
		case eMASTER:
			particleMaster.pos(x,y,z);
			particleMaster.dir(dx,dy,dz);
			particleMaster.color(r,g,b);
			particleMaster.colorChange(dr,dg,db);
			particleMaster.life = life;
			particleMaster.status = status;
			particleMaster.texture = texture;
			break;

		case eVARIANCE:
			particleVariance.pos(x,y,z);
			particleVariance.dir(dx,dy,dz);
			particleVariance.color(r,g,b);
			particleVariance.colorChange(dr,dg,db);
			particleVariance.life = life;
			particleVariance.status = status;
			particleVariance.texture = texture;
			break;

		default:
			// Make sure it's a valid particle
			if (iParticle >= p.size() )
				return false;

			p[iParticle].pos(x,y,z);
			p[iParticle].dir(dx,dy,dz);
			p[iParticle].color(r,g,b);
			p[iParticle].colorChange(dr,dg,db);
			p[iParticle].life = life;
			p[iParticle].status = status;
			p[iParticle].texture = texture;

			break;
	}

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Resets entire engine
//	-	To use properly:
//			1) Setup a "perfect" particle as the first parameter
//			2) Choose how much variance you want on position, color, etc... in
//				the variance particle
//
//	PRE:	particleMaster,	The particle from which all new particles copy
//									their intial information is set.
//			particleVariance,	A particle whose values will be used as variance to
//									the master particle is set.
//			m_bLifeJitter,		Set to true if you want the particles to have a
//									random "starting" position in their life.
//
//	RETURN:	true, on success
//				false, otherwise
//////////////////////////////////////////////////////////////////////////////
bool CParticleEngine::resetAll()
{
	std::vector<CParticle>::iterator itr;

	// Loop through all particles, reseting them
	for (itr = p.begin(); itr < p.end(); itr++)
	{
		if ( !itr->reset(particleMaster, particleVariance) )
			return false;
		if (m_bLifeJitter)
			itr->life = (rand() % particleMaster.life);
	}

	m_bLifeJitter = false;
	
	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Move forward one unit in time, I.E.: Advance the particle engine
// -	Function is unraveled for maximum performance
//
//	ARGS:	fAdjust,	[1.0] A floating point number to adjust the frame rate by.
//						If you send 1.0, that is normal speed.  0.5 means you are
//						running twice speed.  2.0 means it needs to move twice as
//						fast because you are getting half the frame rate.
//
//	RETURN:	true, on success
//				false, otherwise
//////////////////////////////////////////////////////////////////////////////
bool CParticleEngine::advance(float fAdjust)
{
	std::vector<CParticle>::iterator itr;

	if (bGravity)
	{
		// Using gravity
		for (itr = p.begin(); itr < p.end(); itr++)
		{
			// Adjust direction using gravity
			itr->dir += (grav * fAdjust);
			
			// Change position according to direction
			itr->pos += (itr->dir * fAdjust);

			if ( (itr->life -= (1.0f * fAdjust)) <= 0.0f )
				itr->reset(particleMaster, particleVariance);
		}
	}
	else
	{
		// Simple case
		for (itr = p.begin(); itr < p.end(); itr++)
		{
			// Change position according to direction
			// ???TBH itr->pos += (itr->dir * fAdjust);
			itr->pos.x += (itr->dir.x * fAdjust);
			itr->pos.y += (itr->dir.y * fAdjust);
			itr->pos.z += (itr->dir.z * fAdjust);

			if ( (itr->life -= (1.0f * fAdjust)) <= 0.0f )
				itr->reset(particleMaster, particleVariance);
		}
	}

	return true;
}