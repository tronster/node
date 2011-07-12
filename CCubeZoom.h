// CCubeZoom.h: interface for the CCubeZoom class.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __CCUBEZOOM_H_
#define __CCUBEZOOM_H_


#include "CDemoEffect.h"
#include <vector>


// Constants for the cube
#define	fCUBE_START_SCALE	0.001f	// Start scale size of cube.
#define	nMAX_CUBE_DEPTH	5


class CCube
{
	///////////////////////////////////////////////////////////////////////////
	//	MEMBERS
	///////////////////////////////////////////////////////////////////////////
public:
	float x,y,z;					// Position of this cube.
	float rx,ry,rz;				// Rotation of this cube.
	float m_fScale;				// Scale of cube system.
	
	
	///////////////////////////////////////////////////////////////////////////
	//	METHODS
	///////////////////////////////////////////////////////////////////////////
public:
	// Constructor
	CCube()
	{ 
		init();
	}

	// Copy constructor
	CCube(const CCube &copy)
	{ 
		init();
	}

	// Initialize
	void init()
	{
		x	= 0.0f;
		y	= 0.0f;
		z	= 0.0f;
		rx	= (float)(rand() % 360);
		ry	= (float)(rand() % 360);
		rz	= (float)(rand() % 360);
		m_fScale			= fCUBE_START_SCALE;
	}

};



class CCubeZoom : public CDemoEffect  
{
	///////////////////////////////////////////////////////////////////////////
	//	MEMBERS
	///////////////////////////////////////////////////////////////////////////
private:	
	CEnvInfo*			m_pEnvInfo;				// Demo environment infomation.
	float					m_ambientLight[4];
	float					m_diffuseLight[4];
	float					m_lightPos[4];
	unsigned int		m_glDispObject;		// Display list for cubes. //???TBH
	unsigned int		m_glDispStartIndex;	// Hold # for first recurse list.
	GLUquadricObj*		m_pQuadObject;			// OpenGL quardric object, foreground.
	float					m_fMasterScale;		// Scale for complete obj system.
	float					m_fRotateBackY;		// Rotation background value
	float					m_fBGFade;				// Fade value for background
#ifdef _DEBUG
	int					m_nDepth;				// ???TBH debug
#endif // _DEBUG
	
	// Cube rings entry point.
	std::vector<CCube>		m_vCubes;

	// Textures
	unsigned int*	m_punNodeTexture;
   unsigned int   m_nStartTime;

	// Color at different depths
	float	m_fColorArray[3][6];

	// Members which effect the quality of the demo.
	int m_nSphereQuality;

	///////////////////////////////////////////////////////////////////////////
	//	METHODS
	///////////////////////////////////////////////////////////////////////////
public:
	CCubeZoom(CEnvInfo *pEnvInfo);
	virtual ~CCubeZoom();

	bool init();				// initialize function
	bool unInit();				// un-initialize function
   bool start();           // Effect is starting, called before first advanceFrame()
   bool stop();            // Effect is stopping, called after last advanceFrame()
	bool advanceFrame();		// calculate the frame
	bool renderFrame();		// actually drawing a frame

private:	
	// Recursive function to advance the cube(s).
	void advanceChild(int nDepth);
	
	// Recursive function which actually draws the cube(s).
	void renderChild(int nDepth);

	// Actually draws the object at the end of the recursion
	void drawObject(int nDepth);
};

#endif // __CCUBEZOOM_H_