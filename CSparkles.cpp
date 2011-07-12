//////////////////////////////////////////////////////////////////////////////
//
// CSparkles.cpp 
//	Implementation of the CSparkles class
//
//	-	Routine displays red-blue "sparks" spinning from righ to left across
//		the screen.
//	-	A simple use of Tronster's wacky, unoptimized particle engine.
//	-	Tabs set a 3.  (Tronster prefers real tabs, Moby Disk prefers spaces.)
//
//////////////////////////////////////////////////////////////////////////////

#include "CSparkles.h"


// Common iterator types
#define itrSparkType std::vector<CSpark>::iterator 



//////////////////////////////////////////////////////////////////////////////
//
// CONSTRUCTOR
//
//	ARGS:	pEnvInfo,	Point to environment class, which gives us information
//							about the OpenGL/demo environment we're in.
//
//////////////////////////////////////////////////////////////////////////////
CSparkles::CSparkles(CEnvInfo *pEnvInfo) :
	m_pSparkTexture(NULL),
	m_pEnvInfo(pEnvInfo)
{}



//////////////////////////////////////////////////////////////////////////////
//
// DESTRUCTOR
//
//////////////////////////////////////////////////////////////////////////////
CSparkles::~CSparkles()
{
	unInit();
}



bool CSparkles::init()
{	

	// Load textures	
	md::Cimage oImage;
	
	// Background image
	if (!m_pEnvInfo->oMedia.read("spark.png", oImage))
		throw "CSparkles.init()\r\nCould not load spark.png";
	m_pSparkTexture = new unsigned int;
	*m_pSparkTexture = oImage.makeGLTexture(m_pEnvInfo->glWantMipmap,m_pEnvInfo->glWantLinear);
	oImage.destroy();

	// Create # of sparks
	for (int nSparks = 0; nSparks < nNUMBER_OF_SPARKS; nSparks++)
		m_vSpark.push_back(m_pEnvInfo->nDemoQuality);

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//
// Un-initializes textures.
//
//	RETURNS: true,		On success.
//				false,	On failure.
//
//////////////////////////////////////////////////////////////////////////////
bool CSparkles::unInit()
{
	// Remove shadow image.
	if (m_pSparkTexture != NULL)
		glDeleteTextures(1, m_pSparkTexture), m_pSparkTexture = NULL;

	return true;
}



bool CSparkles::start()
{
	m_lStartTime = m_pEnvInfo->getMusicTime();
	return true;
}


bool CSparkles::stop()
{
	// Remove sparkles
	m_vSpark.resize(0);
	return true;
}



bool CSparkles::advanceFrame()
{
	long lCurrentTime = m_pEnvInfo->getMusicTime();
	float fFrameFactor = m_pEnvInfo->fFrameFactor;

	for (	itrSparkType itrSpark = m_vSpark.begin(); 
			itrSpark < m_vSpark.end();
			itrSpark++)
	{
		// Move spark's global position
		itrSpark->x += (itrSpark->dx * fFrameFactor);
		if ((itrSpark->x < -5.0f) && (lCurrentTime - m_lStartTime < 5000))
			itrSpark->init();
		
		// Rotate the sparks
		itrSpark->m_fRotate += 
			(itrSpark->m_fRotateSpeed * fFrameFactor);
		if (itrSpark->m_fRotate > 360.0f)
			itrSpark->m_fRotate -= 360.0f;

		// Let the particle engine do it's thing
		itrSpark->m_PE1.advance(fFrameFactor);
		itrSpark->m_PE2.advance(fFrameFactor);
	}
		
	return true;
}



bool CSparkles::renderFrame()
{
	glPushAttrib(GL_ENABLE_BIT);	// ???TBH - what bit?
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);		
	glDepthMask(GL_FALSE);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);

	glEnable(GL_TEXTURE_2D);

   glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -5.0f);


	glBindTexture(GL_TEXTURE_2D, *m_pSparkTexture);

	for (	itrSparkType itrSpark = m_vSpark.begin();
			itrSpark < m_vSpark.end();
			itrSpark++)
	{
		int nParticle;
		
		glPushMatrix();

			glTranslatef(itrSpark->x, itrSpark->y, itrSpark->z);
			glRotatef(itrSpark->m_fRotate, 0, 0, 1);

			for (	nParticle = 0; 
					nParticle < itrSpark->m_PE1.m_iParticles;
					nParticle++)
			{
				glPushMatrix();		
					glTranslatef(	itrSpark->m_PE1.p[nParticle].pos.x,
										itrSpark->m_PE1.p[nParticle].pos.y,
										itrSpark->m_PE1.p[nParticle].pos.z);
					glColor3f(	itrSpark->m_PE1.p[nParticle].color.r,
									itrSpark->m_PE1.p[nParticle].color.g,
									itrSpark->m_PE1.p[nParticle].color.b);
					
					glBegin(GL_QUADS);
						glTexCoord2f(1.0f, 1.0f); glVertex3f(0.1f,  0.1f, 0.0f);
						glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.1f, 0.1f, 0.0f);
						glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.1f,-0.1f, 0.0f);
						glTexCoord2f(1.0f, 0.0f); glVertex3f(0.1f, -0.1f, 0.0f);
					glEnd();
				glPopMatrix();
			}			

			for (	nParticle = 0; 
					nParticle < itrSpark->m_PE2.m_iParticles;
					nParticle++)
			{
				glPushMatrix();		
					glTranslatef(	itrSpark->m_PE2.p[nParticle].pos.x,
										itrSpark->m_PE2.p[nParticle].pos.y,
										itrSpark->m_PE2.p[nParticle].pos.z);
					glColor3f(	itrSpark->m_PE2.p[nParticle].color.r,
									itrSpark->m_PE2.p[nParticle].color.g,
									itrSpark->m_PE2.p[nParticle].color.b);
					
					glBegin(GL_QUADS);
						glTexCoord2f(1.0f, 1.0f); glVertex3f(0.1f,  0.1f, 0.0f);
						glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.1f, 0.1f, 0.0f);
						glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.1f,-0.1f, 0.0f);
						glTexCoord2f(1.0f, 0.0f); glVertex3f(0.1f, -0.1f, 0.0f);
					glEnd();
				glPopMatrix();
			}			

		glPopMatrix();
	}
	
	glDepthMask(GL_TRUE);
	glPopAttrib();	

	return true;
}
