#pragma warning(disable:4786)
#include <stdio.h>			// Header File For Standard Input/Output
#include <stdlib.h>        // Header has rand()
#include "CCubesACM.h"
#include "loaders3d.h"     // Object and texture loaders header
#include "tesselate3d.h"   // Tesselate 3D objects

using namespace geoGL;
using namespace std;

#define BGSTART      0        // Time for background to fly on
#define BGTIME       10000
#define SYMMOVESTART 7000
#define SYMMOVETIME  5000
#define CUBESTART    7000     // Motion of cubes
#define CUBETIME     38000

#define CUBES_START_POS    50
#define CUBES_MAXDIST      (-50)
#define CUBES_BLEND_DIST   (-30)
#define CUBES_BLEND_LEN    (CUBES_MAXDIST - CUBES_BLEND_DIST)
#define XY_SPACING         5
#define Z_SPACING          5

// Cube positions
fVector3D CCubesACM::cubes_pos[CUBES_POS] = {
   fVector3D(-3,-1,0), fVector3D(-2,-1,0), fVector3D(-1,-1,0), fVector3D(-0.5,-1,0), fVector3D(+0.5f,-1,0), fVector3D(+1,-1,0), fVector3D(+2,-1,0), fVector3D(+3,-1,0),  
   fVector3D(-3,+1,0), fVector3D(-2,+1,0), fVector3D(-1,+1,0), fVector3D(-0.5,+1,0), fVector3D(+0.5f,+1,0), fVector3D(+1,+1,0), fVector3D(+2,+1,0), fVector3D(+3,+1,0),

   // Optional row
   fVector3D(+4,+1,0), fVector3D(+4,-1,0), fVector3D(-4,+1,0), fVector3D(-4,-1,0)
};

// Cube "type" (used for size)
int CCubesACM::cubes_type[CUBES_POS] = {
   0,0,0,1,1,0,0,0,
   0,0,0,1,1,0,0,0,
   // Optional row
   0,0,0,0
};

// Compute animation factor 0..1
static float calcFactor(unsigned int nElapsedTime, unsigned int nStartTime, unsigned int nDuration)
{
   if (nElapsedTime<nStartTime)              return 0;
   if (nElapsedTime>nStartTime+nDuration)    return 1;
   return static_cast<float>(nElapsedTime-nStartTime) / nDuration;
}


CCubesACM::CCubesACM(CEnvInfo *oEnvInfo) : m_oEnvInfo(*oEnvInfo) {}
CCubesACM::~CCubesACM()
{
	unInit();
}



bool CCubesACM::init()
{
   // Only do the recursive texture if quality is >=5
   bUseRecursion = (m_oEnvInfo.nDemoQuality>=5);

   // How big us the recursive texture?  Make it larger in high resolutions
   if (m_oEnvInfo.nWinWidth<1280)
      nRecurseSize = 128;
   else
      nRecurseSize = 256;

   // Number of cubes to draw varies based on quality

   // Quality <4 - remove the outermost set of cubes
   if (m_oEnvInfo.nDemoQuality < 5)
      nCubesPos = CUBES_POS - 4;
   else
      nCubesPos = CUBES_POS;

   // If Quality <3, remove half the rows of cubes
   int nCubesRows;
   if (m_oEnvInfo.nDemoQuality<3)         // Lower quality
   {
      fZSpacing = Z_SPACING * 2.0f;       // Twice as far apart
      nCubesRows = CUBES_ROWS/2;          // Half as many
   } else {
      fZSpacing = Z_SPACING;              // Normal
      nCubesRows = CUBES_ROWS;
   }

   // Compute the total # of cubes
   nCubes = nCubesRows * nCubesPos;

   // Initialize objects, textures, camera...
   initObjects();
   return true;
}

bool CCubesACM::unInit() {
   //???WHG Free the textures here
   //???WHG don't forget the generated images if needed
   return true;
}

bool CCubesACM::start()
{
   initGLstuff();             // Initialize opengl settings

   nTimeStart = m_oEnvInfo.getMusicTime();
   return true;
}

bool CCubesACM::stop()
{
   light0.off();
   return true;
}

//
// Load a bitmap texture
//   - Load BMP
//   - Create mipmapped texture(linear,linear)
//   - Add to geoGL texture list (for loader)
GLuint CCubesACM::loadGLTexture(md::CmediaDuke &mediaDuke, char *filename, char *textureName/*=NULL*/)
{
   GLuint      nTexture;
   md::Cimage  textureImage;

   // Load image, return on failure
   if (!mediaDuke.read(filename,textureImage))
   {
      char temp[200];
      sprintf(temp,"Unable to read %s",filename);
      throw temp;
   }

   // Create texture
	nTexture = textureImage.makeGLTexture(m_oEnvInfo.glWantMipmap, m_oEnvInfo.glWantLinear);
   if (nTexture<=0) {
      char temp[200];
      sprintf(temp,"Unable to create texture %s",filename);
      throw temp;
   }

   // Add texture to map
   if (textureName)
      mapTextures[string(textureName)] = nTexture;

   // Return texture id
   return nTexture;
}

void CCubesACM::LoadGLTextures()
{
   // Load generic PNG textures
   texture_cube2  = loadGLTexture(m_oEnvInfo.oMedia,"ambback.png", "cube2");
   texture_cubeGuy      = loadGLTexture(m_oEnvInfo.oMedia,"ambfront.png","cubeguy");
   texture_cubeGuyMask  = loadGLTexture(m_oEnvInfo.oMedia,"ambfrontmask.png","cubeguymask");
   texture_cubeGuyGlow  = loadGLTexture(m_oEnvInfo.oMedia,"ambfront2.png","cubeguyglow");
   texture_cubeGuyGlowMask  = loadGLTexture(m_oEnvInfo.oMedia,"ambfront2mask.png","cubeguyglowmask");

   imageRecurse.create(nRecurseSize,nRecurseSize,3);
   memset(imageRecurse.data,128,nRecurseSize*nRecurseSize*3);

   texture_recurse = imageRecurse.makeGLTexture(GL_LINEAR,GL_LINEAR);
}

//
// Create objects, load texture, setup video parameters
//
bool CCubesACM::initGLstuff()										// All Setup For OpenGL Goes Here
{
   // Undo tronster's stuff
   glColor3f(1,1,1);
   glEnable(GL_CULL_FACE);
   glDisable(GL_COLOR_MATERIAL);

// OpenGL setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
   glEnable(GL_CULL_FACE);
//   glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glDepthFunc(GL_LEQUAL);								// Allows blends to be drawn
   glEnable(GL_LIGHTING);

   // Setup camera
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
	gluPerspective(45.0f,
      4.0f/3.0f,                          // force 4/3 aspect ratio even if the window is stretched
      2,270.0f);
   glMatrixMode(GL_MODELVIEW);
   cam.position(0,0,0);

   // Light setup
   Light3D::setSceneAmbient(fRGBA(0,0,0,0));
   light0.setLight(1.0f,1.0f,1.0f,1);
   light0.position(0.0f,0.0f,0);
   light0.on();

   return true;
}

bool CCubesACM::initObjects()
{
   // Texture setup
	LoadGLTextures();

   // Load background object
   objBG = Load3SO("data.md/acm_bg.3so",NULL,false);
   FOROBJSET(objBG,obj)
      obj.nTextureID = texture_cube2;
   ENDFOROBJSET
   objBG.rescale(40,40,40);
   objBG.position(0,75,CUBES_MAXDIST);
   objBG.compile();

   // Large cube
   objCube[0] = Load3SO("data.md/cube1tex.3so",NULL,false);
   FOROBJSET(objCube[0],obj)
      obj.nTextureID = bUseRecursion ? texture_recurse : texture_cube2;
   ENDFOROBJSET
   //objCube[0].compile();    //???WHG No compile

   // Small cube
   objCube[1] = Load3SO("data.md/cube.3so",NULL,false);
   objCube[1].rescale(0.5f,0.5f,0.5f);
   FOROBJSET(objCube[1],obj)
      obj.nTextureID = bUseRecursion ? texture_recurse : texture_cube2;
   ENDFOROBJSET
   //objCube[1].compile();    //???WHG No compile

   // Create cubes
   for (int i=0; i<nCubes; i++) {
      int nIndex = i % nCubesPos;
      fTrans[i] = 0;
      objCubes[i] = objCube[cubes_type[nIndex]];
      objCubes[i].position = cubes_pos[nIndex]*XY_SPACING + fVector3D(0,0,-fZSpacing) * (i / nCubesPos + 1);
      objCubes[i].position.z += CUBES_START_POS;
   }

   return true;
}

void CCubesACM::renderZany(GLenum nTextureID)
{
   float fZanyFactor = calcFactor(nElapsedTime,SYMMOVESTART,SYMMOVETIME);

	glBindTexture(GL_TEXTURE_2D, nTextureID);
	glEnable(GL_TEXTURE_2D);

   glPushMatrix();
   glTranslatef(0,25*(1-fZanyFactor),0);
	glBegin(GL_QUADS);
      glNormal3f(0,0,1);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-1.25f, -1.25f, -10.5f);
		
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(+1.25f, -1.25f, -10.5f);
		
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(+1.25f, +1.25f, -10.5f);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-1.25f,+1.25f, -10.5f);
	glEnd();			
   glPopMatrix();
			
	glDisable(GL_TEXTURE_2D);
}

bool CCubesACM::renderFrame()
{
   //
   // If we are drawing the recursive cubes, we need to render twice:
   //   Once for texture, once for visible.
   //
   if (bUseRecursion && nElapsedTime > CUBESTART && nElapsedTime < CUBESTART+CUBETIME)
   {
      // Set viewport for render-to-texture
      glViewport(0, 0, nRecurseSize,nRecurseSize);
      renderFrameInternal();

      // Grab screen image onto a texture
      glBindTexture(GL_TEXTURE_2D, texture_recurse);
      glCopyTexSubImage2D(GL_TEXTURE_2D,0, 0,0,
                          0,0, nRecurseSize,nRecurseSize);
      // Now clear that junk
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Restore viewport and re-render
      glViewport(0,0, m_oEnvInfo.nWinWidth,m_oEnvInfo.nWinHeight);
      renderFrameInternal();
   }
   else
   {
      renderFrameInternal();
   }
   return true;
}

void CCubesACM::renderFrameInternal()
{
   // Setup camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
   cam.execute();

   // Position the lights
   light0.executeLight();

   // Static background
   objBG.call();

   // Main routine time
   if (nElapsedTime > CUBESTART && nElapsedTime < CUBESTART+CUBETIME)
   {
      // Thingy in center
      glMaterialfv(GL_FRONT,GL_AMBIENT,fRGBA(0.0f,0.0f,0.0f,1));
      glMaterialfv(GL_FRONT,GL_DIFFUSE,fRGBA(1.0f,1.0f,1.0f,1));
      glMaterialfv(GL_FRONT,GL_EMISSION,fRGBA(0.0f,0.0f,0.0f,1));
      glMaterialfv(GL_FRONT,GL_SPECULAR,fRGBA(0.0f,0.0f,0.0f,1));

      glPushMatrix();
      glEnable(GL_BLEND);
      glDepthMask(GL_FALSE);
      glBlendFunc(GL_ZERO,GL_SRC_COLOR);
      renderZany(texture_cubeGuyMask);
      glBlendFunc(GL_ONE,GL_ONE);
      renderZany(texture_cubeGuy);
      //Symbol

/* Newew - uses mask and alpha*/
      float narf = sin((nElapsedTime/20000.0f) * 4*PI);
      glMaterialfv(GL_FRONT,GL_DIFFUSE,fRGBA(1.0f,1.0f,1.0f,narf));
      glBlendFunc(GL_ZERO,GL_SRC_COLOR);
      renderZany(texture_cubeGuyGlowMask);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE);
      renderZany(texture_cubeGuyGlow);
/**/

/* New - uses mask
      glBlendFunc(GL_ZERO,GL_SRC_COLOR);
      renderZany(texture_cubeGuyGlowMask);
      glBlendFunc(GL_ONE,GL_ONE);
      renderZany(texture_cubeGuyGlow);
*/
/* just alpha
      glMaterialfv(GL_FRONT,GL_DIFFUSE,fRGBA(1.0f,1.0f,1.0f,1));
      glBlendFunc(GL_SRC_ALPHA,GL_ONE);
      renderZany(texture_cubeGuyGlow);
*/
      glDisable(GL_BLEND);
      glDepthMask(GL_TRUE);
      glPopMatrix();

      // Sets of cubes
      int nObj;
      /* ???WHG No compile
      for (nObj=0; nObj<nCubes; nObj++)
         objCubes[nObj].call();
      */
      // Draw normal cubes
      for (nObj=0; nObj<nCubes; nObj++)
         if (fTrans[nObj]==0)
            objCubes[nObj].execute();
      // Draw transparent cubes
      glDepthMask(GL_FALSE);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      for (nObj=0; nObj<nCubes; nObj++)
         if (fTrans[nObj]!=0)
         {
            FOROBJSET(objCubes[nObj],obj)
               obj.diffuse.a = (1-fTrans[nObj]);
            ENDFOROBJSET
            objCubes[nObj].execute();
            FOROBJSET(objCubes[nObj],obj)
               obj.diffuse.a = 0;
            ENDFOROBJSET
         }
      glDisable(GL_BLEND);
      glDepthMask(GL_TRUE);
   }

   // Debug
   #ifdef _DEBUG
	if (m_oEnvInfo.bShowDebugInfo)
	{
      glMaterialfv(GL_FRONT,GL_EMISSION,fwhite);
      m_oEnvInfo.OglDebug.printf(0,64,0,"Yea baby!");
      glMaterialfv(GL_FRONT,GL_EMISSION,fblack);
   }
   #endif

   glFlush();
}

bool CCubesACM::advanceFrame()
{
   nElapsedTime = m_oEnvInfo.getMusicTime() - nTimeStart;

   float fBGFactor = calcFactor(nElapsedTime,BGSTART,BGTIME);
   objBG.position.y = 65*(1-fBGFactor);

   if (nElapsedTime > CUBESTART && nElapsedTime < CUBESTART+CUBETIME)
   {
      // Move cubes
      int nObj;
      for (nObj=0; nObj<nCubes; nObj++) {
         objCubes[nObj].position -= (fVector3D(0,0,0.1f) * m_oEnvInfo.fFrameFactor);

         if (objCubes[nObj].position.z <= CUBES_MAXDIST)
         {
            objCubes[nObj].position.z -= CUBES_MAXDIST;
            fTrans[nObj] = 0;
         }

         if (objCubes[nObj].position.z <= CUBES_BLEND_DIST)
         {
            float fTemp = (objCubes[nObj].position.z - CUBES_BLEND_DIST) / CUBES_BLEND_LEN;
            fTrans[nObj] = fTemp;
         }
      }
   }

   return true;
}
