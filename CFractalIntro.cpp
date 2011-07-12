#pragma warning(disable:4786)
#include <stdio.h>			// Header File For Standard Input/Output
#include <stdlib.h>        // Header has rand()
#include "CFractalIntro.h"
#include "loaders3d.h"     // Object and texture loaders header
#include "tesselate3d.h"   // Tesselate 3D objects
#include "mduke.h"         // Mediaduke loader

using namespace geoGL;
using namespace std;

#define NUM_SYMBOLS     3     // Number of fading symbols
#define TREE_DEPTH_MIN  7     // Normal tree depth level
#define TREE_DEPTH_VARY 4     // Amount of depth that can vary
#define TREE_DEPTH_MAX  12    // Do not pass this depth or the tree looks too bushy

static float fLeftExtent = 0;
static float fRightExtent = 0;

//////////////////////////////////////////////////////////////////////////////
//	TIMING FOR DEMO EFFECTS
//////////////////////////////////////////////////////////////////////////////
// Normal time
enum {
   eFADEIN_START     = 0,           // Initial fade-in
   eFADEIN_TIME      = 3000,
   eSCROLL_START     = 0,           // Scroll down
   eSCROLL_TIME      = 40000,
   eSYM_START        = 10000,       // Symbols over scroller
   eSYM_TIME         = 20000,
   eGROW_START       = 37000,       // Then grow the tree
   eGROW_TIME        = 17000,
   eWIND_START       = 55000,       // Then the wind on the particles
   eWIND_TIME        = 30000,
   ePART_START       = 70000,       // Then the particles fly around
   ePART_TIME        = 15000,
   ePARTFLY_START    = 85000,       // Then they fly off
   ePARTFLY_TIME     =  5000,
   eBACK_START       = 85000,       // Then the background braks away
   eBACK_TIME        = 5000
};

#define SCROLLDIST   7              // How far to scroll
#define SIGNOF(x)          ((x)<0 ? -1 : 1)

#define WINDX_AMP    (1.0f/2)
#define WINDX_FREQ   (2)
#define WINDY_AMP    (1.0f/50)
#define WINDY_FREQ   (10)

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

// Silly macros for determining timing stuff
// I love this bad macro trick!
#define effectTime(EFFECT)  (nElapsedTime>=e##EFFECT##_START && nElapsedTime<e##EFFECT##_START+e##EFFECT##_TIME)
#define effectFactor(EFFECT)  (calcFactor(nElapsedTime,e##EFFECT##_START,e##EFFECT##_TIME))

/*********************************** CFractalIntro ***********************************/

//
// Load a bitmap texture
//   - Load BMP
//   - Create mipmapped texture(linear,linear)
//   - Add to geoGL texture list (for loader)
GLuint CFractalIntro::loadGLTexture(md::CmediaDuke &mediaDuke, char *filename, char *textureName/*=NULL*/)
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

CFractalIntro::CFractalIntro(CEnvInfo *pEnvInfo) :  m_oEnvInfo(*pEnvInfo) {}
CFractalIntro::~CFractalIntro()
{ 
	unInit();
}

bool CFractalIntro::init()
{
   initObjects();
   initTree();
   fCurrDepth = 0;
   fWindX = fWindY = 0;
   nSymbol = 0;
   fSymFactor = 0;
   fLineScale = m_oEnvInfo.nWinWidth / 1024.0f;
   return true;
}

bool CFractalIntro::unInit()
{
   // Deallocate the background tiles, objects creating it, and the source image
   objCube.clear();
   tileBG.clear();
   oBigBackground.destroy();

   // Deallocate the tree and the points used to grow it
   tree1.clear();
   pPointsGrow.clear();

   // Deallocate symbols
   pSymbols.clear();
   //objSymbol.something();
   return true;
}

bool CFractalIntro::start() {
   nTimeStart = m_oEnvInfo.getMusicTime();
   initGLstuff();
   return true;
}

bool CFractalIntro::stop()
{
   light0.off();
   return true;
   // ???WHG???  Cleanup - You can deallocate those refptrs, make gltiler have a cleanup
}

//
// Load all the textures we use, and some we don't
//
void CFractalIntro::loadGLTextures()
{
   // Load kewl background
   if (!m_oEnvInfo.oMedia.read("fractalintrobg.png",oBigBackground))
      throw "CFractalIntro: Cannot load fractalintrobg.png!";

   // Tile it
   tileBG.setImage(&oBigBackground);
   tileBG.setTileSize(256);
   tileBG.createTiles();

   // Load overlay symbols
   pSymbols.refnew(NUM_SYMBOLS);
   pSymbols[0] = loadGLTexture(m_oEnvInfo.oMedia,"com.png",NULL);
   pSymbols[1] = loadGLTexture(m_oEnvInfo.oMedia,"tronster.png",NULL);
   pSymbols[2] = loadGLTexture(m_oEnvInfo.oMedia,"unkn.png",NULL);
}

//
// Create objects, load texture, setup video parameters
//
void CFractalIntro::initGLstuff()										// All Setup For OpenGL Goes Here
{
   // OpenGL setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
   glEnable(GL_CULL_FACE);
//   glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glDepthFunc(GL_LEQUAL);								// Allows blends to be drawn
   glEnable(GL_LIGHTING);

   // Light setup
   Light3D::setSceneAmbient(fRGBA(0,0,0,0));
   light0.setLight(1.0f,1.0f,1.0f,1);
   light0.position(0.0f,0.0f,5.0f);
   light0.on();

   // Perspective
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
	gluPerspective(60.0f, 4.0f/3.0f, 2,250.0f);  // force 4/3 aspect ratio even if the window is stretched
   glMatrixMode(GL_MODELVIEW);
}

void  CFractalIntro::initTree()
{
   int nTreeDepth = TREE_DEPTH_MIN + (TREE_DEPTH_VARY*m_oEnvInfo.nDemoQuality/5);
   nTreeDepth = TMIN(nTreeDepth,TREE_DEPTH_MAX);

   tree1.init(nTreeDepth,2,2, 3, fVector2D(0,-10));
   tree1.setPoint(0,fVector2D( 0,15), 1,335);
   tree1.setPoint(1,fVector2D(-3,16), 1,25);
   tree1.setPoint(2,fVector2D(-2,14), 4,0);
   /* old tree
   tree1.init(13,2,2, 2, fVector2D(0,-10)); // depth = 13
   tree1.setPoint(0,fVector2D( 0,-17), 1,15);
   tree1.setPoint(1,fVector2D( 0,-17), 1,345);
   */

   tree1.colorGradient(0.4f,0.24f,0.0f, 0.0f,1.0f,0.0f, 0,nTreeDepth+1);
   tree1.render(0);
   tree1.prune();
   tree1.copyToArrays();

   for (int i=0; i<tree1.getNumPoints(); i++) {
      // Scale coordinates to something reasonable - but keep it centered about 0,0
      tree1.getPoints()[i].x/= 60;
      tree1.getPoints()[i].y/= 60;
      // Now move the tree down
      tree1.getPoints()[i].y+= - SCROLLDIST - 2.25f;
   }

   pPointsGrow.refnew(tree1.getNumPoints());
   memcpy(pPointsGrow, tree1.getPoints(), tree1.getNumPoints() * sizeof(fVector2D));
}

void CFractalIntro::initObjects()
{
   // Camera setup
   cam.position(0,0,5);

   // Texture setup
	loadGLTextures();

   // Create cubes for background
   objCube.refnew     (tileBG.getNumTiles());

   int nTile=0;
   int numX = tileBG.getTilesX();
   int numY = tileBG.getTilesY();
   fLeftExtent = -numX - 1;
   fRightExtent= numX + 1;
   for (int x=0; x<numX; x++)
      for (int y=0; y<numY; y++) {
         objCube[nTile]                       = Load3SO("data.md/plane_noglow.3so",NULL,false);
         objCube[nTile].objects[0].nTextureID = tileBG.textures[nTile];
         objCube[nTile].position              = fVector3D(-numX + x*2,-numY + y*2,1);
         nTile++;
      }

   // Object for symbol
   objSymbol = Load3SO("data.md/plane.3so",NULL,false);
   objSymbol.position = fVector3D(0,0,1.01f);
   FOROBJSET(objSymbol,obj)
      obj.color(1,1,1,1);
      obj.ambient(0,0,0,0);
      obj.diffuse(0,0,0,0.0);
      obj.specular(0,0,0,0);
      obj.bSpecularBlend = false;
      obj.emission(0.1f,0.8f,0.3f,1);
      obj.bTransparency = true;
      obj.nTextureID = 0;
   ENDFOROBJSET
}

bool CFractalIntro::renderFrame()
{
   // Setup camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
   cam.execute();

   // Setup light
   light0.executeLight();
         int nlasterr = glGetError();

   // Draw tree and background
   renderBackground();
   renderSymbols();
   renderTree();

   // Debug
   #ifdef _DEBUG
	if (m_oEnvInfo.bShowDebugInfo)
	{
      glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,fRGBA(1,1,1));
      m_oEnvInfo.OglDebug.printf(0,440,0,"nCurrDepth = %f, wind = (%.2f,%.2f)",fCurrDepth,fWindX,fWindY);
      m_oEnvInfo.OglDebug.printf(0,456,0,"nSymbol = %d, fSymFactor = %.2f",nSymbol,fSymFactor);
      glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,fRGBA(0,0,0));
   }
   #endif

   return true;
}

bool CFractalIntro::renderSymbols()
{
   objSymbol.execute();
   return true;
}

bool CFractalIntro::renderBackground()
{
   // The tiles are slightly off so move the background over by 1
   glPushMatrix();
   glTranslatef(1,0,0);

   // Draw the tiles
   for (int nObj=0; nObj<tileBG.getNumTiles(); nObj++)
      objCube[nObj].execute();

   // Restore matrix settings
   glPopMatrix();

   return true;
}

bool CFractalIntro::renderTree()
{
   int nCurrDepth = static_cast<int>(fCurrDepth);

   // Make the tree grow
   if (effectTime(GROW))
   {
      float fCurrFactor= fCurrDepth - nCurrDepth;
      int   nCurrIndex = tree1.getDepthIndices()[nCurrDepth];
      int   nEndIndex  = tree1.getDepthIndices()[nCurrDepth+1];

      for (int i=nCurrIndex; i<nEndIndex; i+=2) {
         fVector2D &startGrow = pPointsGrow[i];
         fVector2D &endGrow   = pPointsGrow[i+1];
         const fVector2D &startOrig = tree1.getPoints()[i];
         const fVector2D &endOrig   = tree1.getPoints()[i+1];

         endGrow = (endOrig-startOrig) * fCurrFactor + startOrig;
      }
   }

   // Draw the tree
   //if (effectTime(GROW) || effectTime(WIND) || effectTime(PART))
   if (nElapsedTime > eGROW_START)
   {
      // Draw the grown points
      float zz = 1-cos(effectFactor(PARTFLY)*PI/2);
      glTranslatef(0,0,1.01f+zz*5);                        // Tree in front of the cubes
      glEnableClientState(GL_VERTEX_ARRAY);           // Draw using a vertex array
      glVertexPointer(2,GL_FLOAT,0,pPointsGrow);      //   of 2D points

      // Use emissive color for this - lighting on lines is always computed weirdly
      glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,fblack);
      glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,fblack);

      // Draw each depth up to the nCurrDepth
      for (int i=0; i<=nCurrDepth; i++)
      {
         glLineWidth((tree1.getMaxDepth() - i) * fLineScale);
         glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,tree1.getColors()[i]/2);
         glDrawArrays(GL_LINES,tree1.getDepthIndices()[i],tree1.getDepthIndices()[i+1] - tree1.getDepthIndices()[i]);
      }

      // Put stuff back
      glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,fblack); // Emissive color back to black
      glLineWidth(1);                                    // Line width back to one
   }

   return true;
}

bool CFractalIntro::advanceSymbols()
{
   if (effectTime(SYM))
   {
      float fEffectFactor = effectFactor(SYM);     // Effect factor 0..1
      nSymbol    = fEffectFactor*NUM_SYMBOLS;      // Symbol number 0..NUM_SYMBOLS-1
                                                   // Symbol animation factor 0..1
      fSymFactor = (fEffectFactor - (static_cast<float>(nSymbol)/NUM_SYMBOLS)) * NUM_SYMBOLS;

      objSymbol.objects[0].nTextureID = pSymbols[nSymbol];
      if (fSymFactor<0.5)
         objSymbol.objects[0].diffuse(0,0,0,fSymFactor*2);
      else
         objSymbol.objects[0].diffuse(0,0,0,(1-fSymFactor)*2);
      objSymbol.position.y = cam.position.y + (fSymFactor - 0.3f);
   }
   return true;
}

bool CFractalIntro::advanceBackground()
{
   if (effectTime(SCROLL))
   {
      float fDist = effectFactor(SCROLL);
      cam.position.y = SCROLLDIST - SCROLLDIST * fDist * 2;
   }

   return true;
}

bool CFractalIntro::advanceTree()
{
   if (effectTime(GROW))
   {
      //fCurrDepth += 0.05 * m_oEnvInfo.fFrameFactor;
      fCurrDepth = effectFactor(GROW) * (tree1.getMaxDepth() - 0.01);
//      if (fCurrDepth>=tree1.getMaxDepth()) 
  //       fCurrDepth=tree1.getMaxDepth()-0.01;
   }

   if (effectTime(WIND) || effectTime(PART) || effectTime(PARTFLY) || effectTime(BACK))
   {
      // Compute animation factor 0..1
      float fWindFactor = effectFactor(WIND) + effectFactor(PART)/2 + effectFactor(PARTFLY)/2;
      // Compute X & Y wind 
      fWindX = sinf(fWindFactor * WINDX_FREQ*PI)*WINDX_AMP;
      fWindY = sinf(fWindFactor * WINDY_FREQ*2*PI)*WINDY_AMP;

      for (int nDepth=0; nDepth<tree1.getMaxDepth(); nDepth++)
      {
         int nStart = tree1.getDepthIndices()[nDepth];
         int nStop  = tree1.getDepthIndices()[nDepth+1];

         for (int i=nStart; i<nStop; i+=2)
         {
            float fInc, k;
            int   d;

            fVector2D &startGrow = pPointsGrow[i];
            fVector2D &startOrig = tree1.getPoints()[i];
            d = tree1.getMaxDepth() - (nDepth+0);
            k = d / static_cast<float>(tree1.getMaxDepth());
            fInc = fWindX - k*fWindX*fWindX * SIGNOF(fWindX);
            if (nDepth == 0)      // Hack - node zero has a still base
               fInc /= 2;
            startGrow.x = startOrig.x + fInc*2;
            startGrow.y = startOrig.y + fWindY*(1-k);

            fVector2D &endGrow   = pPointsGrow[i+1];
            fVector2D &endOrig   = tree1.getPoints()[i+1];
            d = tree1.getMaxDepth() - (nDepth+1);
            k = d / static_cast<float>(tree1.getMaxDepth());
            fInc = fWindX - k*fWindX*fWindX * SIGNOF(fWindX);
            endGrow.x = endOrig.x + fInc*2;
            endGrow.y = endOrig.y + fWindY*(1-k);
         }
      }
     
   }
   return true;
}

bool CFractalIntro::advanceParticles()
{
   // Return if we are not doing particle stuff
   if (!effectTime(PART) && !effectTime(PARTFLY)) return true;

   float fPartFactor    = effectFactor(PART);
   float fPartflyFactor = effectFactor(PARTFLY);

   if (effectTime(PART) || effectTime(PARTFLY))
   {
      for (int i=tree1.getNumPoints()-2; i>=tree1.getNumPoints()*(1-fPartFactor); i-=2)
      {
         fVector2D &start = tree1.getPoints()[i];
         fVector2D &end   = tree1.getPoints()[i+1];
         if (start.x != start.y || start.x != 0)
         {
            float dx = fWindX*m_oEnvInfo.fFrameFactor * WINDX_AMP / 2;
            float dy = fWindY*m_oEnvInfo.fFrameFactor * WINDY_AMP;
            float rx = 0.5f * sin(fPartFactor * 4*2*PI)/100*m_oEnvInfo.fFrameFactor;
            float ry = 0.5f * cos(fPartFactor * 4*2*PI)/100*m_oEnvInfo.fFrameFactor;

            start.x += dx+rx; start.y += dy+ry - m_oEnvInfo.fFrameFactor/100;
            end.x   += dx+rx; end.y   += dy+ry - m_oEnvInfo.fFrameFactor/100;

            // Kill off-screen lines
            if ((start.x > fRightExtent && end.x > fRightExtent) ||
               (start.x < fLeftExtent && end.x < fLeftExtent))
            {
               start.x = start.y = 0;
               end.x = end.y = 0;
            }
         }
      }
   }

   return true;
}

bool CFractalIntro::advanceFrame()
{
   nElapsedTime = m_oEnvInfo.getMusicTime() - nTimeStart;

   if (effectTime(FADEIN))
   {
      float fFadeFactor = effectFactor(FADEIN);
      light0.setLight(fFadeFactor,fFadeFactor,fFadeFactor,1);
   }
   else
   if (effectTime(BACK))
   {
      float fBackFactor = 1-effectFactor(BACK);
      light0.setLight(fBackFactor,fBackFactor,fBackFactor,1);
   }
   else  
      light0.setLight(1,1,1,1);

   advanceSymbols();
   advanceBackground();
   advanceTree();
   advanceParticles();

   return true;
}
