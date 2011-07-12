#pragma warning(disable:4786)
#include <stdio.h>			// Header File For Standard Input/Output
#include <stdlib.h>        // Header has rand()
#include "CShatter.h"
#include "loaders3d.h"     // Object and texture loaders header

using namespace geoGL;
using namespace std;

#define BREAKSTART   0     // Time before tiles start breaking up
#define BREAKTIME    10000

/******************************** Non-members *********************************/

// Random floating point number 0-1
static inline float frand01() { return rand() / static_cast<float>(RAND_MAX); }
// Random floating point number -1 and 1
static inline float frand11() { return (rand()/ static_cast<float>(RAND_MAX)) * 2 - 1; }

// Generic min/max routines
template <class T> static inline T TMAX(T T1, T T2) { return (T1>T2 ? T1 : T2); }
template <class T> static inline T TMIN(T T1, T T2) { return (T1<T2 ? T1 : T2); }

// Compute animation factor 0..1
static float calcFactor(unsigned int nElapsedTime, unsigned int nStartTime, unsigned int nDuration)
{
   if (nElapsedTime<nStartTime)              return 0;
   if (nElapsedTime>nStartTime+nDuration)    return 1;
   return static_cast<float>(nElapsedTime-nStartTime) / nDuration;
}

/*********************************** CShatter ***********************************/

CShatter::CShatter(CEnvInfo *pEnvInfo)
   : m_oEnvInfo(*pEnvInfo)
{
   eShatterType = eSHATTER_TUNNEL;
   szSourceFile = NULL;
   fRotation    = 0;
   bClearDepth  = false;
}

void CShatter::setShatterOptions(char *szSetName, float fSetRotation, int eSetShatterType, bool bSetClear)
{
   eShatterType = eSetShatterType;
   szSourceFile = szSetName;
   fRotation    = fSetRotation;
   bClearDepth  = bSetClear;
}

CShatter::~CShatter() {
	unInit(); 
}

bool CShatter::start()
{
   if (!szSourceFile)
      SnarfScreenTiles();     // Grab the screen at start if not coming from a file
   initGLstuff();             // Initialize opengl settings

   // Setup perspective matrix
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
	gluPerspective(45.0f, 4.0f/3.0f, 0.2f,270.0f);  // force 4/3 aspect ratio even if the window is stretched

   // Start time
   nTimeStart = m_oEnvInfo.getMusicTime();

   return true;
}

bool CShatter::stop()
{
   light0.off();
   return true;
}

bool CShatter::init()
{
   initObjects();             // Initialize objects, textures, camera...
   if (szSourceFile)
      SnarfScreenTiles();     // Load the screen at init if coming from a file
   return true;
}
bool CShatter::unInit() 
{
/* ???wHG CRASHING???
   // Deallocate the background tiles, objects creating it, and the source image
   objCube.clear();
   objCubePos.clear();
   objCubeSpeed.clear();
   tileBG.clear();
   imageBG.destroy();
*/
   return true; 
}

//
// Create objects, load texture, setup video parameters
//
bool CShatter::initGLstuff()										// All Setup For OpenGL Goes Here
{
   // OpenGL setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
   glEnable(GL_CULL_FACE);
//   glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glDepthFunc(GL_LEQUAL);								// Allows blends to be drawn
   glEnable(GL_LIGHTING);

   // Light setup
   for (int i=0; i<8; i++)
      glDisable(GL_LIGHT0+i);

   Light3D::setSceneAmbient(fRGBA(0,0,0,0));
   light0.setLight(1.0f,1.0f,1.0f,1);
   light0.position(0.0f,0.0f,35.0f);
   light0.on();

   return true;
}

bool CShatter::initObjects()
{
   // Camera setup
   cam.position(0,0,27);

	return true;
}

bool CShatter::renderFrame()
{
   glFinish();
   if (bClearDepth)
	   glClear(GL_DEPTH_BUFFER_BIT);

   // Setup camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
   cam.execute();

   // Setup light
   light0.executeLight();

   // Do this stuff 'cuz Tronster's routines change it
	glEnable(GL_CULL_FACE);             // Why is this off?
   glColor3f(1,1,1);                   // Get rid of existing colormaterial settings
   glDisable(GL_COLOR_MATERIAL);       // This messes me up 'cuz I use glMaterial()
   glDisable(GL_BLEND);                // Sparkles leave blending on
   glEnable(GL_LIGHTING);              // Sparkles turn this off too

   // Draw the tiles
   glPushMatrix();
   glScalef(1.45f,1.45f,1);
   for (int nObj=0; nObj<tileBG.getNumTiles(); nObj++)
      objCube[nObj].execute();
   glPopMatrix();

   glEnd();
   glFlush();

   return true;
}

bool CShatter::advanceFrame() {
   // Grab elapsed time
   nElapsedTime = m_oEnvInfo.getMusicTime() - nTimeStart;

   // Breakup motions
   int nObj;

   // This code is inconsistent:
   // For eSHATTER_BACKWARD the "absolute" time model is used.
   // For others, the "incremental" time model is used.

   // Using absolute time is needed here because deviation in position is unnacceptable
   if (nElapsedTime>BREAKSTART && nElapsedTime<=BREAKSTART+BREAKTIME)
   {
      if (eShatterType == eSHATTER_BACKWARD)
      {
         float fFactor = calcFactor(nElapsedTime,BREAKSTART,BREAKTIME);
         glRotatef((1-fFactor)*360,0,0,1);
         for (nObj=0; nObj<tileBG.getNumTiles(); nObj++) {
            objCube[nObj].position.z  = objCubePos[nObj].z * (1-fFactor);
            objCube[nObj].direction = fVector3D(0,45,0) * (1-fFactor); //???WHG 45 should come from setShade...
         }
      }
      else // !eSHATTER_BACKWARD
      {
         for (nObj=0; nObj<tileBG.getNumTiles(); nObj++) 
         {
            objCube[nObj].position += objCubeSpeed[nObj] * m_oEnvInfo.fFrameFactor;
            objCube[nObj].direction+= fVector3D(0,0,fRotation * m_oEnvInfo.fFrameFactor);
         }
      }
   }

   return true;
}

// Snarfs the screen as a 640x480x24 bit Cimage
void CShatter::SnarfScreenImage()
{
   int nWidth, nHeight;                // Screen dimensions
   md::Cimage  imageLarge;             // Image at current screen resolution
   md::Cimage &imageSmall = imageBG;   // Image at 640x480 for tiling

   nWidth = m_oEnvInfo.nWinWidth;
   nHeight= m_oEnvInfo.nWinHeight;

   imageLarge.create(nWidth,nHeight,3);
   imageSmall.create(640,512,3);      // 640x512 fits with a 64x64 tile size

   // Grab the full scree into an image
   glColor3f(1,1,1);
   glReadPixels(0,0, nWidth,nHeight, 
                GL_RGB,GL_UNSIGNED_BYTE,
                imageLarge.data);
   // Now scale it to 640x480
   gluScaleImage(GL_RGB,nWidth,nHeight,GL_UNSIGNED_BYTE, imageLarge.data,
                 640,480,GL_UNSIGNED_BYTE,imageSmall.data);
}

// Snarf the image and place it on tiles
void CShatter::SnarfScreenTiles()
{
   // Texture setup
   if (szSourceFile == NULL)
   {
      // METHOD 1: This will snarf the image from the screen.
      //           It is very slow and causes the music to skip
      SnarfScreenImage();
   }
   else
   {
      // METHOD 2: Grab the image from mediaduke
      if (!m_oEnvInfo.oMedia.read(szSourceFile,imageBG))
      {
         char message[100];
         sprintf(message,"Unable to load shatter image %s",szSourceFile);
         throw message;
      }
   }
   tileBG.setImage(&imageBG);
   tileBG.setTileSize(64);
   tileBG.createTiles();
   imageBG.destroy();         // Don't need this anymore

   // Create cubes for background
   objCube.refnew     (tileBG.getNumTiles());
   objCubeSpeed.refnew(tileBG.getNumTiles());
   objCubePos.refnew  (tileBG.getNumTiles());

   int nTile=0;
   int numX = tileBG.getTilesX();
   int numY = tileBG.getTilesY();
   for (int x=0; x<numX; x++)
      for (int y=0; y<numY; y++) {
         objCubePos[nTile]                    = fVector3D(-numX + x*2,-numY + y*2,0);

         switch(eShatterType) {
            case eSHATTER_RANDOM:
               objCubeSpeed[nTile]            = fVector3D(0,0, frand01() * 0.1f + .05f);
               break;
            case eSHATTER_BACKWARD:
               objCubePos[nTile]   += fVector3D(0,0,frand01() * 70 + 25);
               break;
            case eSHATTER_TUNNEL:
               objCubeSpeed[nTile]            = fVector3D(0,0, (0.5f * (fabs(1.0f * numX/2 - x) + fabs(1.0f * numY/2 - y))) * 0.1f + .05f);
               break;
            default:
               throw "CShatter: Somebody gave me a crazy value!";
         }

         if (eShatterType == eSHATTER_TUNNEL)
         {
            if (x==numX/2 && y==numY/2) {
               objCubeSpeed[nTile] = fVector3D(-0.06f,-0.06f,-0.06f);
            }
            if (x==numX/2+1 && y==numY/2) {
               objCubeSpeed[nTile] = fVector3D(+0.06f,-0.06f,-0.06f);
            }
            if (x==numX/2 && y==numY/2+1) {
               objCubeSpeed[nTile] = fVector3D(-0.06f,+0.06f,-0.06f);
            }
            if (x==numX/2+1 && y==numY/2+1) {
               objCubeSpeed[nTile] = fVector3D(+0.06f,+0.06f,-0.06f);
            }
         }

         objCube[nTile]                       = Load3SO("data.md/cube1tex.3so",NULL,false);
         objCube[nTile].objects[0].nTextureID = tileBG.textures[nTile];
         objCube[nTile].position              = objCubePos[nTile];
         nTile++;
      }

}