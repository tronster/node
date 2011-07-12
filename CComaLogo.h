//////////////////////////////////////////////////////////////////////////////
//
// CComaLogo.h
//	Definition for the CConnect class, CNode class, and CComaLogo class.
//
//	-	Tabs set a 3.  (Tronster prefers real tabs, Moby Disk prefers spaces.)
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __CCOMALOGO_H_
#define __CCOMALOGO_H_


#include "CDemoEffect.h"	// Base class.
#include <vector>				// Standard Template Library's dynamic array.
#include "mduke.h"			// Media Duke, the media loader of champines.


const int NONE = -321;	// Value for a (lack of a) connection to another node.
const int LINK = -444;	// Value for a linking node (one which isn't drawn)



//////////////////////////////////////////////////////////////////////////////
//
//	Class which describes a connector from one node to another.
//
//	- Due to how common x and y are, I don't prefix them with "m_".
//
//////////////////////////////////////////////////////////////////////////////
class CConnect
{
	///////////////////////////////////////////////////////////////////////////
	//	MEMBERS
	///////////////////////////////////////////////////////////////////////////
public:
	int m_nNode;	// ID of node to connect to.
	float x;			// Width position of target connection node.
	float y;			// Height position of target connection node.

	///////////////////////////////////////////////////////////////////////////
	//	METHODS
	///////////////////////////////////////////////////////////////////////////
public:
	CConnect(int _nNode) :
		m_nNode(_nNode),
		x(NONE),
		y(NONE)
	{}
};



//////////////////////////////////////////////////////////////////////////////
//
//	Class which describes a "node".
//
//	-	Due to how common x and y are, I don't prefix them with "m_".
//	-	Connections to other nodes are in the dynamic array of CConnect.
//
//////////////////////////////////////////////////////////////////////////////
class CNode
{
	///////////////////////////////////////////////////////////////////////////
	//	MEMBERS
	///////////////////////////////////////////////////////////////////////////
public:
	bool	m_bAlive;							// Alive, no more growing.
	bool	m_bGrowing;							// Still growing.
	bool	m_bLinkOnly;						// Don't draw this node.
	float m_fSize;								// Size of the node.
	float m_fConnectorColor;				// Color index for connections.
	float x, y;									// Grid position.
	std::vector<CConnect> m_vConnect;	// The node's connections.

	
	///////////////////////////////////////////////////////////////////////////
	//	METHODS
	///////////////////////////////////////////////////////////////////////////
	CNode(float _x,float _y,	int _n1=NONE, int _n2=NONE, int _n3=NONE,
										int _n4=NONE, int _n5=NONE, int _n6=NONE,
										int _n7=NONE, int _n8=NONE) : 
		m_bAlive(false),
		m_bGrowing(false),
		m_fSize(0.001f),
		m_fConnectorColor(0.001f),
		x(_x), y(_y)
	{
		// I'm not very proud of this brute force method, but essentially if
		// a node contains a link, that link is added to it's dynamic array.
		m_bLinkOnly = (x == LINK) ? true : false;
		if (_n1 != NONE)	m_vConnect.push_back(CConnect(_n1));
		if (_n2 != NONE)	m_vConnect.push_back(CConnect(_n2));
		if (_n3 != NONE)	m_vConnect.push_back(CConnect(_n3));
		if (_n4 != NONE)	m_vConnect.push_back(CConnect(_n4));
		if (_n5 != NONE)	m_vConnect.push_back(CConnect(_n5));
		if (_n6 != NONE)	m_vConnect.push_back(CConnect(_n6));
		if (_n7 != NONE)	m_vConnect.push_back(CConnect(_n7));
		if (_n8 != NONE)	m_vConnect.push_back(CConnect(_n8));
	}
};



//////////////////////////////////////////////////////////////////////////////
//
//	-	The effect class for this whole freaking routine.
//	-	Sub-classes off of CDemoEffect which makes us implement some virtuals
//		which are used in loading/unloading/updating/etc...
//
//////////////////////////////////////////////////////////////////////////////
class CComaLogo : public CDemoEffect  
{
	
	///////////////////////////////////////////////////////////////////////////
	//	MEMBERS
	///////////////////////////////////////////////////////////////////////////
private:
	CEnvInfo*	m_pEnvInfo;		// Pointer to the demo's environment object.
	unsigned int m_nStartTime;	// Time (from music) effect starts.
	float m_ambientLight[4];	// Ambient light for the "nodes".
	float m_diffuseLight[4];	// Diffuse light for the "nodes".
	float m_lightPos[4];			// Position of the light.
	float m_fFade;				// Amount of fade at intro.
	float m_fWorldX;			// Left/right logo is on the screen.
	float m_fWorldY;			// Top/bottom logo is on the screen.
	float m_fWorldZ;			// Distance logo is from screen.
	float m_fWorldRotate;	// How is the logo rotated by.
	float m_fBGBrightness;	// Brightness of background texture.
	float m_fShadowOffset;	// Where the shadow is on the screen.
	float m_fShadowScale;	// Scale of the shadow.
	int	m_nStage;			// What part of the effect is currently at.

	GLUquadricObj*			m_pQuadSphere;	// OpenGL construct for sphere.
	std::vector<CNode>	m_vNodes;		// Collection of nodes to draw.

	// Textures
	unsigned int*	m_punBGTexture;	// Big-ass "2" in the background
	unsigned int*	m_punBGShadow;		// Used for the shadow fake.
	unsigned int*	m_punTitle;			// Title screen by Perisoft.

	// Members which effect the quality of the demo.
	int	m_nMinNodeComplexity;	// Minimum rows/columns for a quadric sphere.
	int	m_nMaxNodeComplexity;	// Maximum rows/columns for a quadric sphere.


	///////////////////////////////////////////////////////////////////////////
	//	METHODS
	///////////////////////////////////////////////////////////////////////////
public:
	CComaLogo(CEnvInfo *pEnvInfo);
	virtual ~CComaLogo();

	// The order which these functions are called.
	bool init();				// Initialize function
   bool start();           // Effect is starting, called before first advanceFrame()
	bool advanceFrame();		// calculate the frame
	bool renderFrame();		// actually drawing a frame
   bool stop();            // Effect is stopping, called after last advanceFrame()
	bool unInit();				// Un-initialize function
};


#endif // __CCOMALOGO_H_
