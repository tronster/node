#ifndef __CShatter_h__
#define __CShatter_h__

#ifdef _DEBUG
#include "gldebug.h"
#endif

#include "CDemoEffect.h"
#include "mduke.h"
#include "geometry3d.h"
#include "frustum3d.h"
#include "tiler.h"

class CShatter : public CDemoEffect
{
public:
   //CDemoEffect
   CEnvInfo &                    m_oEnvInfo;

	CShatter(CEnvInfo *);
  ~CShatter();

   enum { eSHATTER_TUNNEL, eSHATTER_RANDOM, eSHATTER_BACKWARD };
   void  setShatterOptions(char *szSetName, float fSetRotation, int eSetShatterType, bool bSetClear);

	bool  advanceFrame();
	bool  renderFrame();
   bool  init();
   bool  unInit();
   bool  start();
   bool  stop();
protected:
   // 3D stuff
   geoGL::Light3D                light0;
   geoGL::Camera3D               cam;

   // Cubes for tiles
   refptr<geoGL::ObjectSet3D>    objCube;          // Cube objects
   refptr<geoGL::fVector3D>      objCubePos;       // Initial positions
   refptr<geoGL::fVector3D>      objCubeSpeed;     // Speed

   // Tiling
   CTilerGL                      tileBG;           // Tiler helper object
   md::Cimage                    imageBG;          // Frozen screen image

   // Shatter options
   float                         fRotation;
   int                           eShatterType;
   bool                          bClearDepth;      // true clears depth buffer before effect
   char *                        szSourceFile;     // Source filename
                                                   // (NULL to use screen shot)

   // Timing
   unsigned int                  nTimeStart,
                                 nElapsedTime;

   // Members
   bool initGLstuff();
   bool initObjects();
   void SnarfScreenImage();
   void SnarfScreenTiles();
};

#endif