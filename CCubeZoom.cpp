// CCubeZoom.cpp: implementation of the CCubeZoom class.
//
//////////////////////////////////////////////////////////////////////////////

#include "CCubeZoom.h"



// Constants for the cube
#define	nCUBE_CHILDREN		6			// Number of children in the cube ring.
#define	fCUBE_RING_START	0.4f		// When to start scaling sub-cubes.
#define	fCUBE_FULL_SCALE	0.5f		// Maximum scale size of cube.
#define	fCUBE_SCALE_SPEED	0.003f	// Speed the cube scales-up at.

#define  nCUBE_START_SHRINKING_TIME 45500		// Time to start fading out.
#define  nBRING_IN_BACKGROUND			 7000		// When background comes in.

// Angle of rotation is 360 degrees minus number of children
#define	fROTATE_ANGLE	(360.0f / nCUBE_CHILDREN)

// Light stuff
const float fLIGHT_FULL_BRIGHT[] = {1.0f, 1.0f, 1.0f, 1.0f};

// Iterators
#define itrCubeType	std::vector<CCube>::iterator

// Macros
// Set colors for each depth
#define ADD_COLOR(n,r,g,b)		\
	m_fColorArray[0][n] = r;	\
	m_fColorArray[1][n] = g;	\
	m_fColorArray[2][n] = b;	



//////////////////////////////////////////////////////////////////////////////
// CONSTRUCTOR
//////////////////////////////////////////////////////////////////////////////
CCubeZoom::CCubeZoom(CEnvInfo *pEnvInfo) :
	m_pEnvInfo(pEnvInfo),		// Get environment info pointer.
	m_punNodeTexture(NULL),		// Reset texture pointer.
	m_pQuadObject(NULL),
	m_fBGFade(0.0f),
	m_fRotateBackY(0.0f)
{
	// Minimum complexity for quadric spheres.
	m_nSphereQuality = (m_pEnvInfo->nDemoQuality + 4);	// Default 9
	if (m_nSphereQuality < 4)
		m_nSphereQuality = 4;
}



CCubeZoom::~CCubeZoom()
{
}



bool CCubeZoom::init()
{ 
	// Setup colors to show for each depth of recursion
	ADD_COLOR(0,	1.0f,	0.6f, 0.6f);
	ADD_COLOR(1,	1.0f,	1.0f, 0.5f);
	ADD_COLOR(2,	0.6f,	1.0f, 0.6f);
	ADD_COLOR(3,	0.3f,	0.4f, 1.0f);
	ADD_COLOR(4,	0.0f,	0.0f, 1.0f);	// ???TBH not used
	ADD_COLOR(5,	0.0f,	0.0f, 1.0f);	// ???TBH not used
	
	// Setup initial lights and positions
	m_ambientLight[0] = 0.2f;
	m_ambientLight[1] = 0.2f;
	m_ambientLight[2] = 0.2f;
	m_ambientLight[3] = 1.0f;

	m_diffuseLight[0] = 0.7f;
	m_diffuseLight[1] = 0.7f;
	m_diffuseLight[2] = 0.7f;
	m_diffuseLight[3] = 1.0f;
	
	m_lightPos[0] = 20.0f;
	m_lightPos[1] = 40.0f;
	m_lightPos[2] = 2.0f;
	m_lightPos[3] = 1.0f;
	

	// Create displays lists for recursion
	m_vCubes.resize(nMAX_CUBE_DEPTH);
	m_glDispStartIndex = glGenLists(nMAX_CUBE_DEPTH);

	// Generate display list
	m_glDispObject = glGenLists(1);

	// Create a new quadric and set our pointer to it
	m_pQuadObject = gluNewQuadric();
	if (m_pQuadObject == 0)
		throw "CCubeZoom.init()\r\nNot enough memory to allocate m_pQuadObject";

	// Set options to how foreground quadric behaves in OpenGL environment
	gluQuadricDrawStyle(		m_pQuadObject, GLU_FILL);
	gluQuadricNormals(		m_pQuadObject, GLU_SMOOTH);
	gluQuadricOrientation(	m_pQuadObject, GLU_OUTSIDE);
	gluQuadricTexture(		m_pQuadObject, GL_TRUE);

	// Load textures	
	md::Cimage oImage;
	if (!m_pEnvInfo->oMedia.read("spheretex.png", oImage))
		throw "CCubeZoom.init()\r\nCould not load spheretex.png";

	m_punNodeTexture = new unsigned int;

	*m_punNodeTexture = oImage.makeGLTexture(m_pEnvInfo->glWantMipmap,m_pEnvInfo->glWantLinear);

	oImage.destroy();

	// To increase performace, I have the quadric rendered in a display list.
	// I have yet to find the time to see if this provides a significant
	// (if any) speed increase.
	glNewList(m_glDispObject, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, *m_punNodeTexture);
		gluSphere(
			m_pQuadObject,			// Quadric identifier
			1.0,						// radius
			m_nSphereQuality,		// horizontal components
			m_nSphereQuality);	// verticle componetns
	glEndList();

	glEnable(GL_NORMALIZE);

	return true;
}



bool CCubeZoom::unInit()
{ 
	// Kill display list for single object
	if (m_glDispObject != 0)
		glDeleteLists(m_glDispObject,1);

	// Kill display list(s) for recurssion
	if (m_glDispStartIndex != 0)
		glDeleteLists(m_glDispStartIndex, nMAX_CUBE_DEPTH);
	
	// Remove node image.
	if (m_punNodeTexture != NULL)
		glDeleteTextures(1, m_punNodeTexture), m_punNodeTexture = NULL;

	// Kill foreground sphere.
	if (m_pQuadObject != NULL)
		gluDeleteQuadric(m_pQuadObject), m_pQuadObject = NULL;

	return true;
}



bool CCubeZoom::start()
{ 
   m_nStartTime = m_pEnvInfo->getMusicTime();
	m_fMasterScale = 1.0f;

	return true;
}



bool CCubeZoom::stop()
{ 
	return true;
}



bool CCubeZoom::advanceFrame()
{ 
	float fElapseTime = (m_pEnvInfo->getMusicTime() - m_nStartTime);
	
	m_fRotateBackY += (0.5f * m_pEnvInfo->fFrameFactor);
	if (m_fRotateBackY > 360.0f) m_fRotateBackY -= 360.0f;
	
	advanceChild(0);

	// Background
	if ( fElapseTime > nCUBE_START_SHRINKING_TIME)
	{
		m_fBGFade -= 0.02f * m_pEnvInfo->fFrameFactor;
		if (m_fBGFade < 0.0f)
			m_fBGFade = 0.0f;

	} else if ( fElapseTime > nBRING_IN_BACKGROUND)
	{
		m_fBGFade += 0.005f * m_pEnvInfo->fFrameFactor;
		if (m_fBGFade > 1.0f)
			m_fBGFade = 1.0f;
	}

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
//	- Performs a recurrsion N levels deep, updating the member information.
//
//////////////////////////////////////////////////////////////////////////////
void CCubeZoom::advanceChild(int nDepth)
{
   // If we are not ending the routine, then scale up
   if ((m_pEnvInfo->getMusicTime() - m_nStartTime) < nCUBE_START_SHRINKING_TIME)
	{   	
		// Scale up the cube.
   	m_vCubes[nDepth].m_fScale += (fCUBE_SCALE_SPEED * m_pEnvInfo->fFrameFactor);

   } else {

		// Shrink the whole system
		m_fMasterScale -= (fCUBE_SCALE_SPEED * m_pEnvInfo->fFrameFactor);
		if (m_fMasterScale < 0.001)
			m_fMasterScale = 0.001;	
   }

	// Clamp scale value to maximum.
	if (m_vCubes[nDepth].m_fScale > fCUBE_FULL_SCALE)	
		m_vCubes[nDepth].m_fScale = fCUBE_FULL_SCALE;
   else
	// Clamp scale value to minimum
	if (m_vCubes[nDepth].m_fScale < fCUBE_START_SCALE)	
		m_vCubes[nDepth].m_fScale = fCUBE_START_SCALE;

	// If big enough, recurse on cube ring.
	if (	(m_vCubes[nDepth].m_fScale >= fCUBE_RING_START) && 
			(nDepth < nMAX_CUBE_DEPTH - 1) )
	{
		// Recurse
		advanceChild(nDepth + 1);
	}

	// Rotate the cube
	m_vCubes[nDepth].rx += (1.0f * m_pEnvInfo->fFrameFactor);
	m_vCubes[nDepth].ry += (1.3f * m_pEnvInfo->fFrameFactor);
	m_vCubes[nDepth].rz += (1.6f * m_pEnvInfo->fFrameFactor);

	// Adjust the rotation
	if (m_vCubes[nDepth].rx > 360.0f) 
		m_vCubes[nDepth].rx -= 360.0f;
	if (m_vCubes[nDepth].ry > 360.0f)
		m_vCubes[nDepth].ry -= 360.0f;
	if (m_vCubes[nDepth].rz > 360.0f)
		m_vCubes[nDepth].rz -= 360.0f;

}



bool CCubeZoom::renderFrame()
{ 
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glLoadIdentity();


	glDisable(GL_CULL_FACE);
//	glFrontFace(GL_CCW);

	// ???TBH Do the light once if possible
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT,		m_ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,		m_diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION,	m_lightPos);
	glEnable(GL_LIGHT0);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);	
	glEnable(GL_TEXTURE_2D);	

	// Translate camera
	glTranslatef(0.0f, 0.0f, -5.0f);

	glPushMatrix();

		// Scale the system
		if (m_fMasterScale != 1.0f)
			glScalef(m_fMasterScale, m_fMasterScale,m_fMasterScale);
	
		renderChild(0);
		glCallList(m_glDispStartIndex);
	glPopMatrix();

	// Render background	
	glRotatef(m_fRotateBackY, 0.0f, 1.0f, 0.0f);

	glShadeModel(GL_SMOOTH);
	glDisable(GL_COLOR_MATERIAL);	
	glDisable(GL_TEXTURE_2D);	
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	if ((m_pEnvInfo->getMusicTime() - m_nStartTime) > nBRING_IN_BACKGROUND)
	{
		for(float fIndex = 0.0f; fIndex < 360.0f; fIndex += 20.0f)
		{
			glPushMatrix();
			   glColor3f(0.4f * m_fBGFade, 0.0f, 0.5f * m_fBGFade);
				glRotatef(fIndex, 0.0f, 1.0f, 0.0f);
				glTranslatef(0.0f, 0.0f, -10.0f);
				glBegin(GL_QUADS);
					glVertex3f(1.0f, 1.0f, 0.0f);
					glVertex3f(1.0f, -1.0f, 0.0f);
					glVertex3f(-1.0f, -1.0f, 0.0f);
					glVertex3f(-1.0f, 1.0f, 0.0f);
				glEnd();
				
				glColor3f(0.3f * m_fBGFade , 0.0f, 0.35f * m_fBGFade);
				
				glPushMatrix();
					glTranslatef(0.0f, 5.0f, 0.0f);
					glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
					glBegin(GL_QUADS);
						glVertex3f(1.0f, 1.0f, 0.0f);
						glVertex3f(1.0f, -1.0f, 0.0f);
						glVertex3f(-1.0f, -1.0f, 0.0f);
						glVertex3f(-1.0f, 1.0f, 0.0f);
					glEnd();
				glPopMatrix();
				glPushMatrix();
					glTranslatef(0.0f, -5.0f, 0.0f);
					glRotatef(-20.0f, 1.0f, 0.0f, 0.0f);
					glBegin(GL_QUADS);
						glVertex3f(1.0f, 1.0f, 0.0f);
						glVertex3f(1.0f, -1.0f, 0.0f);
						glVertex3f(-1.0f, -1.0f, 0.0f);
						glVertex3f(-1.0f, 1.0f, 0.0f);
					glEnd();
				glPopMatrix();
			glPopMatrix();
		}
	}


#ifdef _DEBUG
	if (m_pEnvInfo->bShowDebugInfo)
	{
	   glColor3f(0.9f, 0.9f, 1.0f);
	   m_pEnvInfo->OglDebug.printf(0, 48, 0, "Scales: %4.4f %4.4f %4.4f %4.4f",
		   m_vCubes[0].m_fScale, m_vCubes[1].m_fScale,
		   m_vCubes[2].m_fScale, m_vCubes[3].m_fScale);
	   m_pEnvInfo->OglDebug.printf(0, 64, 0, "Z: %4.4f %4.4f %4.4f %4.4f",
		   m_vCubes[0].z, m_vCubes[1].z, m_vCubes[2].z, m_vCubes[3].z);
	   m_pEnvInfo->OglDebug.printf(0, 80, 0, "Depth: %d", m_nDepth);
	   
	   static int error = GL_NO_ERROR;
	   if (glGetError() != GL_NO_ERROR)
		   error = glGetError();
	   m_pEnvInfo->OglDebug.printf(0, 96, 0, "Error: %d", error);
   }
#endif // _DEBUG

	return true;
}



void CCubeZoom::renderChild(int nDepth)
{
	if (nDepth > 3)
		return;

	if (	(m_vCubes[nDepth].m_fScale >= fCUBE_RING_START) && 
			(nDepth < nMAX_CUBE_DEPTH - 1) )
	{
		glPushMatrix();

			renderChild(nDepth + 1);

			glNewList(m_glDispStartIndex + nDepth, GL_COMPILE);

				// Render the center object
				drawObject(nDepth);

				// Render the orbiting objects
				for (int nChild = 0; nChild < nCUBE_CHILDREN; nChild++)
				{
					glPushMatrix();
						glRotatef(fROTATE_ANGLE * nChild, 0.0f, 1.0f, 0.0f);
						glTranslatef(0.0f, 0.0f, -4.5f);
						glCallList(m_glDispStartIndex + nDepth + 1);
					glPopMatrix();
				}
			glEndList();	

		glPopMatrix();

	} else {	
	
		glNewList(m_glDispStartIndex + nDepth, GL_COMPILE);
			drawObject(nDepth);	
		glEndList();

#ifdef _DEBUG
		m_nDepth = nDepth;
#endif // _DEBUG
	}

	glColor3f(1.0f, 1.0f, 1.0f);
}



void CCubeZoom::drawObject(int nDepth)
{
	glScalef(m_vCubes[nDepth].m_fScale, 
		m_vCubes[nDepth].m_fScale,
		m_vCubes[nDepth].m_fScale);
	
	glRotatef(m_vCubes[nDepth].rx, 1.0f, 0.0f, 0.0f);
	glRotatef(m_vCubes[nDepth].ry, 0.0f, 1.0f, 0.0f);
	glRotatef(m_vCubes[nDepth].rz, 0.0f, 0.0f, 1.0f);

	glColor3f(
		m_fColorArray[0][nDepth],
		m_fColorArray[1][nDepth],
		m_fColorArray[2][nDepth]	);

	glCallList(m_glDispObject);
}
