//////////////////////////////////////////////////////////////////////////////
//
// CSparkles.h
//	Definition for the CSpark class, and CSparkles class.
//
//	-	Tabs set at 3.  (Tronster prefers real tabs, Moby Disk prefers spaces.)
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __CSPARKLES_H_
#define __CSPARKLES_H_


#include "CDemoEffect.h"		// Base class for demo effects.
#include <vector>					// Standard Template Library's dynamic array.
#include "mduke.h"				// Media Duke, the media loader of champions.
#include "CParticleEngine.h"	// Tronster's particle engine class.


#define nNUMBER_OF_SPARKS		12		// Number of sparks to fly across screen.
#define fSPARKLE_START			8.0f	// Offset around where sparks start.


// Generates a random floating point number 0..1
static inline float frand01()
	{ return rand() / static_cast<float>(RAND_MAX); }

// Generates a random floating point number -1..1 
static inline float frand11()
	{ return (rand()/ static_cast<float>(RAND_MAX)) * 2 - 1; }



//////////////////////////////////////////////////////////////////////////////
//
//	-	An object which is made up of two instances of particle engines.
//	-	In "Node" one side is setup to be red, the other blue, and they "spin"
//		from the right side, to the left side of the screen.
//
//////////////////////////////////////////////////////////////////////////////
class CSpark
{
	///////////////////////////////////////////////////////////////////////////
	//	MEMBERS
	///////////////////////////////////////////////////////////////////////////
private:
	// Particles per engine are set based on the demo's quality switch.
	int m_nParticlesPerEngine;
	
	// Quality setting.  (Internally stored as the CEnvInfo isn't passed in.)
	int m_nQuality;
	
public:
	float x,y,z;					// Spark's location.
	float dx;						// Spark's direction.
	float m_fRotate;				// Angle of rotation for spark.
	float m_fRotateSpeed;		// Speed at which the spark is rotating.
	CParticleEngine m_PE1;		// The blue side of the spark.
	CParticleEngine m_PE2;		// The red side of the spark.


	///////////////////////////////////////////////////////////////////////////
	//	METHODS
	///////////////////////////////////////////////////////////////////////////
public:
	// Constructors
	CSpark()							{ init();			}
	CSpark(int nQuality)			{ init(nQuality); }
	
	// Copy Constructor
	CSpark(const CSpark &copy)	{ init(copy.m_nQuality); }

	// Destructor
	~CSpark()
	{	}
	
	// While we have a default value on quality hre , we should technically 
	// always be called with the constructor that has the proper quality value.
	void init(int nQuality = 0)
	{		
		// Establish the number of particles to show from quality setting.
		m_nQuality = nQuality;
		m_nParticlesPerEngine = 5 + (10 * m_nQuality);
		if (m_nParticlesPerEngine < 5)
			m_nParticlesPerEngine = 5;

		// Set the default start position of a "spark".
		x = fSPARKLE_START + (frand11() * 3.0f);
		y = (frand11() * 2.5f);
		z = 0.0f;
		
		// Set the default travel speed and rotation of our "spark".
		dx = -0.05f;
		m_fRotate = rand() % 360;
		m_fRotateSpeed = (rand() % 4) + 1;

		// This effect didn't call for gravity effecting the sparks.
//		m_PE1.grav(0.0f, -0.005f, 0.0f);
//		m_PE1.setState(true, CParticleEngine::eGRAVITY);		

		// Create the particles
		m_PE1.init(m_nParticlesPerEngine, true);
		
		// Set the initial values of a particle
		m_PE1.setParticle(CParticleEngine::eMASTER,
								0.0f,	 0.0f,  0.0f,		// Start location.
								0.0f,  0.02f, 0.0f,		// Direction of movement.
								0.4f,  0.4f,  1.0f,		// Color
								0.0f,	 0.0f,  0.0f,		// Color change (not impl)
								90,		0,	  0);			// life, status, texture

		// Set the amount of variance for the aboce values
		m_PE1.setParticle(CParticleEngine::eVARIANCE,
								0.0f,	 0.0f,	0.0f,		// Start location variance.
								0.03f, 0.0f,	0.03f,	// Movement varience.
								0.4f,  0.4f,	0.0f,		// Color varience.
								0.0f,	 0.0f,	0.0f,		// Color change (not impl)
								20,		0,		0);		// varience: life, status, tex

		m_PE1.resetAll();	// Reset this particle system.		


		// This effect didn't call for gravity effecting the sparks.
//		m_PE2.grav(0.0f, -0.005f, 0.0f);
//		m_PE2.setState(true, CParticleEngine::eGRAVITY);
		
		// Create the particles
		m_PE2.init(m_nParticlesPerEngine, true);
		
		// Set the initial values of a particle
		m_PE2.setParticle(CParticleEngine::eMASTER,
								0.0f,	 0.0f,  0.0f,		// Start location.
								0.0f, -0.02f, 0.0f,		// Direction of movement.
								1.0f,  0.4f,  0.4f,		// Color
								0.0f,	 0.0f,  0.0f,		// Color change (not impl)
								90,		0,	  0);			// life, status, texture

		// Set the amount of variance for the above values
		m_PE2.setParticle(CParticleEngine::eVARIANCE,
								0.0f,	 0.0f,	0.0f,		// Start location variance.
								0.03f, 0.0f,	0.03f,	// Movement varience.
								0.0f,  0.4f,	0.4f,		// Color varience.
								0.0f,	 0.0f,	0.0f,		// Color change (not impl)
								20,		0,		0);		// varience: life, status, tex
		
		m_PE2.resetAll();	// Reset this particle system.

	}

};



//////////////////////////////////////////////////////////////////////////////
//
//	-	The demo effect which maintains all of the "sparks" traveling across
//		the screen.
//
//////////////////////////////////////////////////////////////////////////////
class CSparkles : public CDemoEffect  
{
	///////////////////////////////////////////////////////////////////////////
	//	MEMBERS
	///////////////////////////////////////////////////////////////////////////
private:
	CEnvInfo*	m_pEnvInfo;				// Demo environment info class.
	std::vector<CSpark> m_vSpark;		// Collection of sparks.
	unsigned int*	m_pSparkTexture;	// OpenGL Texture for spark.
	long m_lStartTime;					// Time sparkle effect was started.


	///////////////////////////////////////////////////////////////////////////
	//	METHODS
	///////////////////////////////////////////////////////////////////////////
public:
	CSparkles(CEnvInfo *pEnvInfo);
	virtual ~CSparkles();

	bool init();				// Initialize function.
	bool unInit();				// Un-initialize function.
   bool start();           // Effect is starting.
   bool stop();            // Effect is stopping.
	bool advanceFrame();		// Calculate the frame.
	bool renderFrame();		// Actually drawing a frame.

};

#endif // __CSPARKLES_H_
