//////////////////////////////////////////////////////////////////////////////
//
// CComaLogo.cpp 
//	Implementation of the CComaLogo class
//
//	-	Routine displays a still bitmap which says "Node"
//	-	The "o" in "Node" is one sphere in a collection of lines and spheres
//		which become the word "Coma"
// -	A [fake] shadow effect appears on a bitmap of a "2" behind "Coma"
//	-	Tabs set a 3.  (Tronster prefers real tabs, Moby Disk prefers spaces.)
//
//////////////////////////////////////////////////////////////////////////////

#include "CComaLogo.h"
#include <cmath>


// Common iterator types (these are STL constructs)
#define itrNodeType		std::vector<CNode>::iterator 
#define itrConnectType	std::vector<CConnect>::iterator

// Node related
#define GROW_NEW_NODES	1500		// Millisecs until next nodes start growing.
#define NODE_MIN_SIZE	0.0001f	// Minimum size of a sphere.
#define NODE_MAX_SIZE	0.3f		// Maximum size of a sphere.

// Scene related.
#define fSCENE_FAR_PLANE 100.0f			// Distance of far clipping plane.
#define fSCENE_MAX_BG_BRIGHTNESS 0.2	// How bright the background becomes.
#define fSCENE_STEP_BG_BRIGHTNESS 0.004f	// Size of steps to brighten bg.
#define fSHADOW_RANGE 10.0f				// Starting position of shadow bitmap.

// Special stages
#define STAGE_SCREEN_SHOT	-1		// Used during development for a screen shot.
#define STAGE_NO_MOVE		-2		// Used during development for testing.

// Timer related
#define fBEGIN_MOVE_TIME 5000		// Delay from start until movement begins.



//////////////////////////////////////////////////////////////////////////////
//
// CONSTRUCTOR
//
//	ARGS:	pEnvInfo,	Pointer to environment class, which gives us information
//							about the OpenGL/demo environment we're in.
//
//////////////////////////////////////////////////////////////////////////////
CComaLogo::CComaLogo(CEnvInfo *pEnvInfo) : 
	m_pEnvInfo(pEnvInfo),		// Get environment info pointer.
	m_pQuadSphere(NULL),			// Reset quadric.
	m_punTitle(NULL),				// Reset what will hold the title texture.
	m_punBGTexture(NULL),		// Reset what will hold the background texture.
	m_punBGShadow(NULL)			// Reset what will hold the shadow texture.
{
	// Determin quality

	// Minimum complexity for quadric spheres.
	m_nMinNodeComplexity = (m_pEnvInfo->nDemoQuality + 3);
	if (m_nMinNodeComplexity < 3)
		m_nMinNodeComplexity = 3;

	// Maximum complexity for quadric spheres.
	m_nMaxNodeComplexity = (m_pEnvInfo->nDemoQuality + 15);
	if (m_nMaxNodeComplexity < 4)
		m_nMaxNodeComplexity = 4;
}



//////////////////////////////////////////////////////////////////////////////
//
// DESTRUCTOR
//
//////////////////////////////////////////////////////////////////////////////
CComaLogo::~CComaLogo()
{
	unInit();
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Sets up lights for the scene.
// -	Loads textures.
//	-	Allocates OpenGL quadric for the "node" spheres.
//	-	Originally I was going to read points from a text file, but with the
//		"Coma 2" party less than 2 weeks away, I opted to use this brute force
//		way of entering in the points.  Yes, I know it's hacky.  I should have
//		at least written a macro.
//
//	RETURNS: true,		On success.
//				false,	On failure.
//
//	THROWS:	char *
//
//////////////////////////////////////////////////////////////////////////////
bool CComaLogo::init()
{
	// Setup initial lights and positions
	m_ambientLight[0] = 0.2f;
	m_ambientLight[1] = 0.2f;
	m_ambientLight[2] = 0.2f;
	m_ambientLight[3] = 1.0f;

	m_diffuseLight[0] = 0.7f;
	m_diffuseLight[1] = 0.7f;
	m_diffuseLight[2] = 0.7f;
	m_diffuseLight[3] = 1.0f;
	
	m_lightPos[0] = fSHADOW_RANGE * 8;
	m_lightPos[1] = 60.0f;
	m_lightPos[2] = 10.0f;
	m_lightPos[3] = 1.0f;
	

	// Format: x, y, connecting nodes list...
	// Good luck, I did this on graph paper.

	// "O"
	// [[1 - 5]]
	m_vNodes.push_back(CNode(0,0,2,3));
	m_vNodes.push_back(CNode(0,1,5));
	m_vNodes.push_back(CNode(-1,1,2,4,5));
	m_vNodes.push_back(CNode(-2,2,6,14));
	m_vNodes.push_back(CNode(0,2,6,7));
	// [[6 - 10]]
	m_vNodes.push_back(CNode(-1,2,3));
	m_vNodes.push_back(CNode(1,2,8,19));
	m_vNodes.push_back(CNode(1,3,5,9));
	m_vNodes.push_back(CNode(2,4,10));
	m_vNodes.push_back(CNode(3,3,11,12,13));
	// [[11 - 15]]
	m_vNodes.push_back(CNode(2,3,8,9,12));
	m_vNodes.push_back(CNode(2,2,7,8,19));
	m_vNodes.push_back(CNode(4,4,28,61));	// to "M"
	m_vNodes.push_back(CNode(-3,3,15));
	m_vNodes.push_back(CNode(-4,4,16,37));	// to "C"
	// [[16 - 20]]
	m_vNodes.push_back(CNode(-3,5,17));
	m_vNodes.push_back(CNode(-2,6,18,21));
	m_vNodes.push_back(CNode(-2,5,16,20));
	m_vNodes.push_back(CNode(1,1,1,2,5));
	m_vNodes.push_back(CNode(-1,5,17,21,22));
	// [[21 - 25]]
	m_vNodes.push_back(CNode(-1,6,26,27));
	m_vNodes.push_back(CNode(-2,4,14,24,23,16,18));
	m_vNodes.push_back(CNode(-3,4,14,15,16));
	m_vNodes.push_back(CNode(-2,3,4,14,25));
	m_vNodes.push_back(CNode(-1,3,4,5,6,22));
	// [[26 - 30]]
	m_vNodes.push_back(CNode(-1,7,17));
	m_vNodes.push_back(CNode(0,6,20,26,33));
	m_vNodes.push_back(CNode(3,5,9,29,30));
	m_vNodes.push_back(CNode(3,4,9,10,13));
	m_vNodes.push_back(CNode(2,5,9,31));
	// [[31 - 35]]
	m_vNodes.push_back(CNode(1,5,9,27,33,32));
	m_vNodes.push_back(CNode(2,6,28,30,33));
	m_vNodes.push_back(CNode(1,6,34));
	m_vNodes.push_back(CNode(1,7,27,32,35,36));
	m_vNodes.push_back(CNode(0,8,26,36));
	// [[36 - 40]]
	m_vNodes.push_back(CNode(0,7,26,27));

	// "C"
	m_vNodes.push_back(CNode(LINK, LINK, 38, 59));
	m_vNodes.push_back(CNode(-6,6,39,42,47));
	m_vNodes.push_back(CNode(-5,7,40,41));
	m_vNodes.push_back(CNode(-6,7,38,41,42));

	// [[41 - 45]]
	m_vNodes.push_back(CNode(-6,8,42));
	m_vNodes.push_back(CNode(-7,7,43,44));
	m_vNodes.push_back(CNode(-7,6,38,44));
	m_vNodes.push_back(CNode(-8,6,45,46,47));
	m_vNodes.push_back(CNode(-9,5,46));

	// [[46 - 50]]
	m_vNodes.push_back(CNode(-8,5,50));
	m_vNodes.push_back(CNode(-7,5,46,43));
	m_vNodes.push_back(CNode(-10,4,45));
	m_vNodes.push_back(CNode(-9,4,45,48));
	m_vNodes.push_back(CNode(-8,4,45,47,49,51,52,53));

	// [[51 - 55]]
	m_vNodes.push_back(CNode(-9,3,48,49,52));
	m_vNodes.push_back(CNode(-8,3));
	m_vNodes.push_back(CNode(-7,3,52));
	m_vNodes.push_back(CNode(-8,2,51,52,53));
	m_vNodes.push_back(CNode(-7,2,53,54,56));

	// [[56 - 60]]
	m_vNodes.push_back(CNode(-6,2,53,57,58,59));
	m_vNodes.push_back(CNode(-7,1,54,55));
	m_vNodes.push_back(CNode(-6,1,57));
	m_vNodes.push_back(CNode(-5,1,58,60));
	m_vNodes.push_back(CNode(-6,0,57,58));

	// "M"
	// [[61 - 65]]
	m_vNodes.push_back(CNode(LINK,LINK,62));
	m_vNodes.push_back(CNode(6,6,63,81));
	m_vNodes.push_back(CNode(7,5,64,73));
	m_vNodes.push_back(CNode(6,4,65));
	m_vNodes.push_back(CNode(7,3,66,71,74));
	
	// [[66 - 70]]
	m_vNodes.push_back(CNode(6,2,67));
	m_vNodes.push_back(CNode(7,1,68,70,72,75));
	m_vNodes.push_back(CNode(6,0,69));
	m_vNodes.push_back(CNode(7,0,67));
	m_vNodes.push_back(CNode(8,0,69,76));

	// [[71 - 75]]
	m_vNodes.push_back(CNode(8,4,63,78,80));
	m_vNodes.push_back(CNode(8,2,65,77,79));
	m_vNodes.push_back(CNode(6,5,62,64));
	m_vNodes.push_back(CNode(6,3,64,66));
	m_vNodes.push_back(CNode(6,1,66,68));

	// [[76 - 80]]
	m_vNodes.push_back(CNode(8,1,67,72));
	m_vNodes.push_back(CNode(8,3,65,71));
	m_vNodes.push_back(CNode(8,5,63,83));
	m_vNodes.push_back(CNode(7,2,65,66,67));
	m_vNodes.push_back(CNode(7,4,63,64,65));

	// [[81 - 85]]
	m_vNodes.push_back(CNode(7,7,85,121));			// to "A"
	m_vNodes.push_back(CNode(7,6,62,63,81));
	m_vNodes.push_back(CNode(8,6,63,81,82,84));
	m_vNodes.push_back(CNode(8,7,81,85,86));
	m_vNodes.push_back(CNode(8,8,86));

	// [[86 - 90]]
	m_vNodes.push_back(CNode(9,7,83,91));//m_vNodes.push_back(CNode(9,7,83,92));
	m_vNodes.push_back(CNode(9,6,83,86,88));
	m_vNodes.push_back(CNode(9,5,83,89));
	m_vNodes.push_back(CNode(10,4,90,96));
	m_vNodes.push_back(CNode(10,5,88,91));

	// [[91 - 95]]	// Ack, two nodes not really needed for a good looking M
	m_vNodes.push_back(CNode(10,6,86,87,88,94));
	m_vNodes.push_back(CNode(LINK,LINK));	//	m_vNodes.push_back(CNode(10,7,93,91));
	m_vNodes.push_back(CNode(LINK,LINK));	//	m_vNodes.push_back(CNode(10,8,86,94));
	m_vNodes.push_back(CNode(11,7,92,97,99));
	m_vNodes.push_back(CNode(11,6,91,94,97));

	// [[96 - 100]]
	m_vNodes.push_back(CNode(11,5,90,91,95));
	m_vNodes.push_back(CNode(12,6,96,104));
	m_vNodes.push_back(CNode(12,7,94,97,100));
	m_vNodes.push_back(CNode(12,8,98));
	m_vNodes.push_back(CNode(13,7,97,99,102));
	
	// [[101 - 105]]
	m_vNodes.push_back(CNode(13,6,97,100));
	m_vNodes.push_back(CNode(14,6,101,103));
	m_vNodes.push_back(CNode(14,5,104,108));
	m_vNodes.push_back(CNode(13,5,101,102,106));
	m_vNodes.push_back(CNode(12,5,97,104));

	// [[106 - 110]]
	m_vNodes.push_back(CNode(12,4,105,111));
	m_vNodes.push_back(CNode(13,4,104,106));
	m_vNodes.push_back(CNode(14,4,104,107,109));
	m_vNodes.push_back(CNode(14,3,114));
	m_vNodes.push_back(CNode(13,3,106,107,108,109,111));

	// [[111 - 115]]
	m_vNodes.push_back(CNode(12,3,112));
	m_vNodes.push_back(CNode(12,2,110,116,117));
	m_vNodes.push_back(CNode(13,2,110,112,114));
	m_vNodes.push_back(CNode(14,2,110));
	m_vNodes.push_back(CNode(14,1,114,120));
	
	// [[116 - 120]]
	m_vNodes.push_back(CNode(13,1,113,114,115));
	m_vNodes.push_back(CNode(12,1,116,118));
	m_vNodes.push_back(CNode(12,0,116,119));
	m_vNodes.push_back(CNode(13,0,116,120));
	m_vNodes.push_back(CNode(14,0,116));

	// "A"
	// [[121 - 125]]
	m_vNodes.push_back(CNode(LINK,LINK,122));
	m_vNodes.push_back(CNode(16,1,123,127));
	m_vNodes.push_back(CNode(17,0,125,126));
	m_vNodes.push_back(CNode(19,4,153,154));
	m_vNodes.push_back(CNode(18,1));

	// [[126 - 130]]
	m_vNodes.push_back(CNode(17,1,122,125));
	m_vNodes.push_back(CNode(16,2,132));
	m_vNodes.push_back(CNode(17,2,122,125,126,127,130,131,132));
	m_vNodes.push_back(CNode(18,2,125,128));
	m_vNodes.push_back(CNode(18,3,124,129,135));

	// [[131 - 135]]
	m_vNodes.push_back(CNode(17,3,130,132,134));
	m_vNodes.push_back(CNode(16,3,133));
	m_vNodes.push_back(CNode(16,4,134,138));
	m_vNodes.push_back(CNode(17,4,130,132,135,136,137,138));
	m_vNodes.push_back(CNode(18,4,124));

	// [[136 - 140]]
	m_vNodes.push_back(CNode(18,5,135,139));
	m_vNodes.push_back(CNode(17,5,136));
	m_vNodes.push_back(CNode(16,5,137));
	m_vNodes.push_back(CNode(17,6,137,138,141));
	m_vNodes.push_back(CNode(18,6,136,139,141));

	// [[141 - 145]]
	m_vNodes.push_back(CNode(18,7,142));
	m_vNodes.push_back(CNode(19,8,143,145));
	m_vNodes.push_back(CNode(19,7,141,145));
	m_vNodes.push_back(CNode(19,6,136,140,141,143));
	m_vNodes.push_back(CNode(20,7,144,147));

	// [[146 - 150]]
	m_vNodes.push_back(CNode(20,6,144,145,148));
	m_vNodes.push_back(CNode(21,6,146,149));
	m_vNodes.push_back(CNode(20,5,144,147));
	m_vNodes.push_back(CNode(21,5,148,152));
	m_vNodes.push_back(CNode(22,5,147,149,152));

	// [[151 - 155]]
	m_vNodes.push_back(CNode(22,4,150,156));
	m_vNodes.push_back(CNode(21,4,148,151,154,156));
	m_vNodes.push_back(CNode(20,4,148,152,154));
	m_vNodes.push_back(CNode(20,3,158,159));
	m_vNodes.push_back(CNode(21,3,152,154,156));

	// [[156 - 160]]
	m_vNodes.push_back(CNode(22,3,157));
	m_vNodes.push_back(CNode(22,2,162));
	m_vNodes.push_back(CNode(21,2,155,156,157,159,161));
	m_vNodes.push_back(CNode(20,2,160));
	m_vNodes.push_back(CNode(20,1,158,163));

	// [[161 - 165]]
	m_vNodes.push_back(CNode(21,1,160));
	m_vNodes.push_back(CNode(22,1,158,161,163));
	m_vNodes.push_back(CNode(21,0,161));

	
	// Now that node information is filled, translate the target node values
	// into x,y coordinates.
	
	// Loop through nodes.  (Here is where an iterator comes in handy.)
	for (	itrNodeType itrNode = m_vNodes.begin();
			itrNode < m_vNodes.end();
			itrNode++)
	{

		// Loop through connections for the current node.
		for ( itrConnectType itrConnect = 
					itrNode->m_vConnect.begin();
				itrConnect < itrNode->m_vConnect.end();
				itrConnect++)
		{
			// Set the connection coordinates.
			itrConnect->x = m_vNodes[itrConnect->m_nNode - 1].x;
			itrConnect->y = m_vNodes[itrConnect->m_nNode - 1].y;		
		}
	
	}

	
	// Create a new quadric and set our pointer to it
	m_pQuadSphere = gluNewQuadric();
	if (m_pQuadSphere == 0)
		throw "CComaLogo.init()\r\nNot enough memory to allocate m_pQuadSphere";

	// Set options to how quadric behaves in OpenGL environment
	gluQuadricDrawStyle(	m_pQuadSphere,		GLU_FILL);
	gluQuadricNormals(	m_pQuadSphere,		GLU_SMOOTH);
	gluQuadricOrientation(m_pQuadSphere,	GLU_OUTSIDE);
	gluQuadricTexture(	m_pQuadSphere,		GL_FALSE);
	
	// Load textures	
	
	md::Cimage oImage;	// Create a "media duke" image object
	
	// Load title image as an OpenGL texture.
	if (!m_pEnvInfo->oMedia.read("nodelogo.png", oImage))
		throw "CComaLogo.init()\r\nCould not load nodelogo.png";
	m_punTitle = new unsigned int;
	*m_punTitle = oImage.makeGLTexture(m_pEnvInfo->glWantMipmap,m_pEnvInfo->glWantLinear);
   if (*m_punTitle==-1)
      throw "CComaLogo.init()\r\nUnable to create texture for nodelogo.png";
	oImage.destroy();

	// Load background image as an OpenGL texture.
	if (!m_pEnvInfo->oMedia.read("coma2bg.png", oImage))
		throw "CComaLogo.init()\r\nCould not load coma2bg.png";
	m_punBGTexture = new unsigned int;
	*m_punBGTexture = oImage.makeGLTexture(m_pEnvInfo->glWantMipmap,m_pEnvInfo->glWantLinear);
   if (*m_punBGTexture==-1)
      throw "CComaLogo.init()\r\nUnable to create texture for coma2bg.png";
	oImage.destroy();

	// Load shadow image as an OpenGL texture.
	if (!m_pEnvInfo->oMedia.read("nodeshadow.png", oImage))
		throw "CComaLogo.init()\r\nCould not load nodeshadow.png";
	m_punBGShadow = new unsigned int;
	*m_punBGShadow = oImage.makeGLTexture(m_pEnvInfo->glWantMipmap,m_pEnvInfo->glWantLinear);
   if (*m_punBGShadow==-1)
      throw "CComaLogo.init()\r\nUnable to create texture for nodeshadow.png";
	oImage.destroy();

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	-	Called the first time we enter this routine (after init() ).
//	-	Because this resets the project matrix, it assumes to be coming from a
//		blank screen, and that no other effects will be running when this
//		function is called
//
//	RETURNS: true, Always.
//
//////////////////////////////////////////////////////////////////////////////
bool CComaLogo::start()
{ 
	// Grab time
	m_nStartTime	= m_pEnvInfo->getMusicTime();
	
	m_nStage			= 0;			// Reset effect stage to beginning.
	m_fWorldX		= -0.33f;	// World coordinates for the logo's translation.
	m_fWorldY		= 0.01f;		// ...
	m_fWorldZ		= -3.0f;		// ...
	m_fWorldRotate = -70.0f;	// Set the starting rotate angle for logo.
	m_fFade			= 0.0f;		// Start intro bitmap at pure black.
	m_fBGBrightness= 0.0f;		// Background brightness is black.
	
	// Starting offset of shadow.
	m_fShadowOffset= -fSHADOW_RANGE;	
	
	// Starting scale of shadow image.
	m_fShadowScale = 1.0f + (fSHADOW_RANGE / 60.0f);

	// Reset projection matrix
	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(
		45.0f,										// field of view
      4.0f/3.0f,                          // force 4/3 aspect ratio even if the window is stretched
		0.20f,	      							// "near" plane to camera
		fSCENE_FAR_PLANE);						// "far" plane to camera

	// Reset model view
	glMatrixMode(GL_MODELVIEW);

	return true; 
}



//////////////////////////////////////////////////////////////////////////////
//
//	Called the last time we run this routine (before uninit() ).
//
//	RETURNS: true, Always.
//
//////////////////////////////////////////////////////////////////////////////
bool CComaLogo::stop()
{
	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
// Un-initializes textures and quadric(s).
//
//	RETURNS: true,	Always.
//
//////////////////////////////////////////////////////////////////////////////
bool CComaLogo::unInit()
{
	// Remove title image.
	if (m_punTitle != NULL)
		glDeleteTextures(1, m_punTitle), m_punTitle = NULL;

	// Remove shadow image.
	if (m_punBGShadow != NULL)
		glDeleteTextures(1, m_punBGShadow), m_punBGShadow = NULL;

	// Remove background image.
	if (m_punBGTexture != NULL)
		glDeleteTextures(1, m_punBGTexture), m_punBGTexture = NULL;
	
	// Kill the OpenGL quadric sphere.
	if (m_pQuadSphere != NULL)
		gluDeleteQuadric(m_pQuadSphere), m_pQuadSphere = NULL;
	
	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
// -	Calculate the movement in the frame.
//	-	According to what "stage" we are in, update the nodes.
//
//	RETURNS: true,	Always.
//
//////////////////////////////////////////////////////////////////////////////
bool CComaLogo::advanceFrame()
{
	// Establish a starting point for effect
	static int	nLastUpdate =	(m_pEnvInfo->getMusicTime());
	int			nCurrent =		(m_pEnvInfo->getMusicTime());

	
	// Translation and stage changes
	switch(m_nStage)
	{
	// Hold still and don't start growing, as we're showing the title screnn.
	case 0:
		m_vNodes[0].m_bAlive		= true;
		m_vNodes[0].m_bGrowing	= false;
		m_vNodes[0].m_fSize		= NODE_MAX_SIZE;		
		if ((m_pEnvInfo->getMusicTime() - m_nStartTime) > fBEGIN_MOVE_TIME)
		{
			// Once this starts growing, all the others will follow.
			m_vNodes[0].m_bGrowing = true;
			
			m_nStage++;
		}	

//		m_nStage = STAGE_SCREEN_SHOT;	// ???TBH - used to take screen shot
		break;


	// Begin movement and "growing" of the nodes.
	case 1:
		m_fWorldX += (0.001f * m_pEnvInfo->fFrameFactor);
		m_fWorldY -= (0.005f * m_pEnvInfo->fFrameFactor);
		m_fWorldZ -= (0.03f * m_pEnvInfo->fFrameFactor);
		m_fWorldRotate += (0.05f * m_pEnvInfo->fFrameFactor);
		if (m_fWorldZ < -15.0f)
			m_nStage++;
		break;


	// Sames as above, but adjust the panning / zooming.
	case 2:
		m_fWorldX -= (0.004f * m_pEnvInfo->fFrameFactor);
		m_fWorldY -= (0.004f * m_pEnvInfo->fFrameFactor);
		m_fWorldZ -= (0.03f * m_pEnvInfo->fFrameFactor);
		m_fWorldRotate += (0.06f * m_pEnvInfo->fFrameFactor);
		if (m_fWorldZ < -20.0f)
			m_nStage++;
		break;


	// Again, continue but with slightly different panning / zooming.
	case 3:
		m_fWorldX -= (0.007f * m_pEnvInfo->fFrameFactor);
		m_fWorldY -= (0.001f * m_pEnvInfo->fFrameFactor);
		m_fWorldZ -= (0.028f * m_pEnvInfo->fFrameFactor);
		m_fWorldRotate += (0.07f * m_pEnvInfo->fFrameFactor);
		if (m_fWorldRotate > 0.0f)
			m_fWorldRotate = 0.0f;
		if (m_fWorldZ < -26.0f)
			m_nStage++;
		break;


	// Again-again
	case 4:
		// World
		m_fWorldX -= (0.013f * m_pEnvInfo->fFrameFactor);
		m_fWorldY += (0.001f * m_pEnvInfo->fFrameFactor);
		m_fWorldZ -= (0.02f * m_pEnvInfo->fFrameFactor);	
		m_fWorldRotate += (0.08f * m_pEnvInfo->fFrameFactor);
		if (m_fWorldRotate > 0.0f)
			m_fWorldRotate = 0.0f;
		
		if (m_fWorldZ < -32.0f)
			m_nStage++;
		break;

	// "Coma" logo is done growing / moving, sweep shadow across background.
	case 5:
		// Brightness
		m_fBGBrightness += (fSCENE_STEP_BG_BRIGHTNESS * m_pEnvInfo->fFrameFactor);
		if (m_fBGBrightness > fSCENE_MAX_BG_BRIGHTNESS)
			m_fBGBrightness = fSCENE_MAX_BG_BRIGHTNESS;

		// Shadow
		m_fShadowOffset += (0.15f * m_pEnvInfo->fFrameFactor);
		if (m_fShadowOffset > fSHADOW_RANGE)
			m_fShadowOffset = fSHADOW_RANGE;
		m_lightPos[0] = -m_fShadowOffset * 8;
		m_fShadowScale = 1.0f + (fabs(m_fShadowOffset) / 60.0f);
		break;


	case STAGE_NO_MOVE:
		break;


	// Used for taking a screen shot of the initial position.  This was so
	// that Perisoft could draw the rest of the intro picture with the
	// proper positioning of the "O".
	case STAGE_SCREEN_SHOT:
		m_fWorldX =	 -0.33f;
		m_fWorldY =  0.02f;
		m_fWorldZ =  -3.0f;
		break;


	// As far as I know, it's impossible to be here.
	default:
		break;
	}


	// Node updating.
	// Because most all of the above stages rely on the nodes growing, I broke
	// out this second check of the stage.
	switch(m_nStage)
	{
	case 0:
	case STAGE_SCREEN_SHOT:
	case STAGE_NO_MOVE:
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:

		// See if it's time to start the next chain of nodes
		if ((nCurrent - nLastUpdate) > GROW_NEW_NODES)
		{
			// Create a vector (it's dynamic array) which will hold the ID
			// numbers of the nodes which are going to begin growing.
			std::vector<int> vNewGrowers;

			nLastUpdate = nCurrent;
			
			// Go through nodes.  Stop growth on currently growing nodes, and
			// begin growth on child nodes.
			for (	itrNodeType itrNode = m_vNodes.begin();
					itrNode < m_vNodes.end();
					itrNode++)
			{
				if (itrNode->m_bGrowing)
				{					
					// Found a growing node, activate it's children.
					for (	itrConnectType itrConnect = itrNode->m_vConnect.begin();
							itrConnect < itrNode->m_vConnect.end();
							itrConnect++)
					{
						// If child node is not already alive, active.
						if ( !m_vNodes[itrConnect->m_nNode - 1].m_bAlive )
							vNewGrowers.push_back(itrConnect->m_nNode);

					} // For connects

					// Stop the growing of this node, and mark alive.
					itrNode->m_bGrowing = false;

				} // If growing

			} // For nodes

			// Go through the collection of nodes which we determined are ready
			// to start growing.
			for (	std::vector<int>::iterator itr = vNewGrowers.begin();
					itr < vNewGrowers.end();
					itr++)
			{
				// Index off of the ID # (minus 1 because vectors are 0 based),
				// and set the default values to start the node growing
				m_vNodes[*itr - 1].m_bAlive = true;
				m_vNodes[*itr - 1].m_bGrowing = true;
				m_vNodes[*itr - 1].m_fSize = NODE_MIN_SIZE;
				m_vNodes[*itr - 1].m_fConnectorColor = 0.001f;
			}

		} else {

			// Change a growing / living node's properties
			for (	itrNodeType itrNode = m_vNodes.begin();
					itrNode < m_vNodes.end();
					itrNode++)
			{
				// Update node variables
				if (itrNode->m_fSize < NODE_MAX_SIZE)
					itrNode->m_fSize += (0.002f * m_pEnvInfo->fFrameFactor);
				
				if (itrNode->m_fConnectorColor < 0.8f)
					itrNode->m_fConnectorColor += (0.007f * m_pEnvInfo->fFrameFactor);

			} // For nodes
			
		} // If-else update

		break;
	
	// Again, as far as I know, it's impossible to be here.	
	default:
		break;
	}	

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
// Actually draw the scene.
//
//	RETURNS: true,		On success.
//				false,	On failure.
//
//////////////////////////////////////////////////////////////////////////////
bool CComaLogo::renderFrame()
{
	unsigned int nElapseTime= m_pEnvInfo->getMusicTime() - m_nStartTime;
	int nComplexity			= 
		static_cast<int>(m_nMaxNodeComplexity - (abs(m_fWorldZ)/2.0f));
	
	// Clamp complexity if it's about to get too low.
	if (nComplexity < m_nMinNodeComplexity)
		nComplexity = m_nMinNodeComplexity;

	glLoadIdentity();			// Reset the world for rotations / transformations.
	glEnable(GL_CULL_FACE);	// Don't draw polygons we can't see.
	glFrontFace(GL_CCW);		// "Front face" polygons drawn counter-clockwise.


	// Draw title screen
	if (m_nStage == 0 || m_nStage == 1)	
	{
		glDisable(GL_LIGHTING);		
		glEnable(GL_TEXTURE_2D);

		// Fade up or down depending on time
		if (nElapseTime < (fBEGIN_MOVE_TIME / 2.0f))
		{			
			// Fade up.
			m_fFade += (0.1f * m_pEnvInfo->fFrameFactor);
			if (m_fFade > 1.0f)
				m_fFade = 1.0f;
		
		} else {
			// Fade down.
			m_fFade -= (0.01f * m_pEnvInfo->fFrameFactor);
			if (m_fFade < 0.0f)
				m_fFade = 0.0f;
		}

		// Set the title screen's brightness and draw.
		
		glColor3f(m_fFade, m_fFade, m_fFade);
		glBindTexture(GL_TEXTURE_2D, *m_punTitle);		
		
		// Title screen is just a square.
		glBegin(GL_QUADS);	
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-26.1f, -31.4f, -46.5f);
			
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(25.7f, -31.4f, -46.5f);
			
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(25.7f, 17.0f, -46.5f);

			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-26.1f, 17.0f, -46.5f);
		glEnd();			
				
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
	}
	

	// Bring in the background.
	if (m_nStage > 3)
	{
		// Background image should not be effected by light.
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
		glDisable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);

		// Set the color of the "2" background to it's brightness.  This will be
		// black for most of the routine.
		glColor3f(m_fBGBrightness, m_fBGBrightness, m_fBGBrightness);

		// Draw main background
		
		glBindTexture(GL_TEXTURE_2D, *m_punBGTexture);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-25.0f, -25.0f,  -45.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 25.0f, -25.0f,  -45.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 25.0f,  25.0f,  -45.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-25.0f,  25.0f,  -45.0f);
		glEnd();

		// Draw shadow on top of background.
		
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);			

		// Coordinates for height of the shadow
		float fShadowCoord = (26.0f * m_fShadowScale) + (60.0f / -m_fWorldZ);

		glBlendFunc(GL_ONE,GL_ONE);		

		glBindTexture(GL_TEXTURE_2D, *m_punBGShadow);
		glBegin(GL_QUADS);	
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-25.0f + m_fShadowOffset, -fShadowCoord, -44.9f);
			
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(25.0f + m_fShadowOffset, -fShadowCoord, -44.9f);
			
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(25.0f + m_fShadowOffset, fShadowCoord, -44.9f);

			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-25.0f + m_fShadowOffset, fShadowCoord, -44.9f);
		glEnd();

		// (Kluge) Put a white square on either side of background texture,
		// because the blending.  I wish I had enough time how to do the
		// blending properly so there would have been more contrast in the
		// background images.
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);	
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-75.0f + m_fShadowOffset, -fShadowCoord, -44.9f);
			
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(-25.0f + m_fShadowOffset, -fShadowCoord, -44.9f);
			
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(-25.0f + m_fShadowOffset, fShadowCoord, -44.9f);

			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-75.0f + m_fShadowOffset, fShadowCoord, -44.9f);
		glEnd();
		glBegin(GL_QUADS);	
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(25.0f + m_fShadowOffset, -fShadowCoord, -44.9f);
			
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(75.0f + m_fShadowOffset, -fShadowCoord, -44.9f);
			
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(75.0f + m_fShadowOffset, fShadowCoord, -44.9f);

			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(25.0f + m_fShadowOffset, fShadowCoord, -44.9f);
		glEnd();

		glDisable(GL_BLEND);
	}

	// Draw the "Coma" logo

	// The logo is effected by lighting.
	glEnable(GL_LIGHTING);	
	glLightfv(GL_LIGHT0, GL_AMBIENT,		m_ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,		m_diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION,	m_lightPos);
	glEnable(GL_LIGHT0);

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	
	glLineWidth(2.0f);
	glTranslatef(m_fWorldX, m_fWorldY, m_fWorldZ);
	glRotatef(m_fWorldRotate, 1.0f, 0.0f, 0.0f);

	// Loop through all the nodes.
	for(	itrNodeType itrNode = m_vNodes.begin();
			itrNode < m_vNodes.end();
			itrNode++)
	{		
		// Only render node if it is suppose to exist on the screen.
		if (	itrNode->m_bAlive && !itrNode->m_bLinkOnly)				
		{
			// Draw a node
			glPushMatrix();
				glTranslatef(itrNode->x, itrNode->y, 0.0f);	// Move to position.
				glColor3f(1.0f, 0.8f, 0.2f);						// Set color.
				
				gluSphere(												// Perform draw.
					m_pQuadSphere,			// Quadrid identifier
					itrNode->m_fSize,		// radius
					nComplexity,			// horizontal components
					nComplexity);			// verticle componetns			
			glPopMatrix();

			// Setup color of the connectors between the nodes.			
			glColor3f(
				itrNode->m_fConnectorColor,
				itrNode->m_fConnectorColor,
				0.0f);
		
			// Don't start drawing if display title logo
			if (nElapseTime > fBEGIN_MOVE_TIME)
			{
				// Loop through all connectors for this node.
				for(	itrConnectType itrConnect = 
							itrNode->m_vConnect.begin();
						itrConnect < itrNode->m_vConnect.end();
						itrConnect++)
				{
					// Check if we should draw this connection.  If it's of type
					// "LINK" then it should not be drawn, because it's purpose
					// is to connect other letters of "COMA" to each other.
					if (itrConnect->x != LINK)
					{
						// Draw connector
						glPushMatrix();
							glBegin(GL_LINES);				

								// Start line at center of current node.
								glVertex3f(itrNode->x, itrNode->y, 0.0f);
								
								// End line at target node.
								glVertex3f(itrConnect->x, itrConnect->y, 0.0f);

							glEnd();
						glPopMatrix();
					
					} // if drawable connection

				} // for connectors
			
			} // if title showing

		} // if node is alive

	} // for nodes

#ifdef _DEBUG	
	if (m_pEnvInfo->bShowDebugInfo)
	{
	   glDisable(GL_LIGHTING);
	   glColor3f(1.0f, 1.0f, 1.0f);
	   m_pEnvInfo->OglDebug.printf(1,48,0,"stage: %d   complexity: %d",
			m_nStage,nComplexity);
	   m_pEnvInfo->OglDebug.printf(1,64,0,"world: %f4, %f4, %f4",
			m_fWorldX, m_fWorldY, m_fWorldZ);
   }
#endif

	return true;
}
