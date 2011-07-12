//////////////////////////////////////////////////////////////////////////////
//
//	cParticle.cpp
//	Todd "Tronster" Hartley
//
//	2000.07.30
//	-	Defines a particle for a particle engine
//	-	geoGL by Moby Disk (http://www.mobydisk.com)
//
//////////////////////////////////////////////////////////////////////////////

#include "CParticle.h"

// Random floating point number -1 and 1 (from Mobydisk)
static inline float frand11() \
	{ return rand() / static_cast<float>(RAND_MAX) - 0.5f; }

// Use to do nothing
const int NOOP = 1;



//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
CParticle::CParticle(float x, float y, float z) :
		pos(x, y, z), life(0)
{
}


		
//////////////////////////////////////////////////////////////////////////////
// Copy Constructor
//////////////////////////////////////////////////////////////////////////////
CParticle::CParticle(const CParticle &copy)
{
	pos	= copy.pos;
	life	= copy.life;
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Reset the particle
//
//	ARGS:	master,		the particle from which initial information is copied
//			variance,	amount of variance to produce from the copy
//			bGiveLife,	[true] if the particle should be alive
//
//
//////////////////////////////////////////////////////////////////////////////
bool CParticle::reset(	CParticle &master,
								CParticle &variance,
								bool bGiveLife )
{
	// Set particle position then jitter by variance
	pos.x = master.pos.x + ((frand11() * variance.pos.x) / 2);
	pos.y = master.pos.y + ((frand11() * variance.pos.y) / 2);
	pos.z = master.pos.z + ((frand11() * variance.pos.z) / 2);

	// Set particle direction then jitter by variance
	dir.x = master.dir.x + ((frand11() * variance.dir.x) / 2);
	dir.y = master.dir.y + ((frand11() * variance.dir.y) / 2);
	dir.z = master.dir.z + ((frand11() * variance.dir.z) / 2);

	// Set particle color then jitter by variance
	color.r = master.color.r + ((frand11() * variance.color.r) / 2);
	color.g = master.color.g + ((frand11() * variance.color.g) / 2);
	color.b = master.color.b + ((frand11() * variance.color.b) / 2);

	// Set particle colorChange then jitter by variance
	colorChange.r = master.colorChange.r + ((frand11() * variance.colorChange.r) / 2);
	colorChange.g = master.colorChange.g + ((frand11() * variance.colorChange.g) / 2);
	colorChange.b = master.colorChange.b + ((frand11() * variance.colorChange.b) / 2);


// ??TBH - TODO: life and texture need to properly obtain a value according to the variance


	// (If desired) set a particle life then jitter by variance
	bGiveLife ?	
		(life = master.life + (rand() % variance.life) ) :
		NOOP;

	// Set particle texture
	texture = master.texture; // ???TBH  + ( (rand() % variance.texture) - (variance.texture / 2) );
	

	return true;
}
