#ifndef __CCubesACM_h__
#define __CCubesACM_h__

#ifdef _DEBUG
#include "gldebug.h"
#endif

#include "geometry3d.h"
#include "flythru3d.h"
#include "frustum3d.h"
#include "mduke.h"
#include "CDemoEffect.h"

class CCubesACM : public CDemoEffect
{
public:
   //CDemoEffect
	CCubesACM(CEnvInfo *);
  ~CCubesACM();

	bool  advanceFrame();
	bool  renderFrame();
   bool  init();
   bool  unInit();
   bool  start();
   bool  stop();

private:
   enum { CUBES_POS = 20,                       // # of unique cube positions
          CUBES_ROWS= 10,                       // # of rows of cubes
          MAX_CUBES = CUBES_POS*CUBES_ROWS };   // Max possible # of cubes

   static geoGL::fVector3D cubes_pos[CUBES_POS];
   static int              cubes_type[CUBES_POS];

   geoGL::ObjectSet3D      objCube[2];
   geoGL::ObjectSet3D      objCenter;
   geoGL::ObjectSet3D      objBG;
   geoGL::ObjectSet3D      objCubes[MAX_CUBES];
   float                   fTrans[MAX_CUBES];
   geoGL::TextureID        texture_cubeGuy, texture_cubeGuyMask, texture_cubeGuyGlow, texture_cubeGuyGlowMask;

   md::Cimage              imageRecurse;

   // Quality settings
   bool                    bUseRecursion;       // true if we use the recursive texture
   int                     nCubes,              // Actual # of cubes
                           nCubesPos,           // Actual # cube positions
                           fZSpacing,           // Spacing between rows of cubes
                           nRecurseSize;        // Size of the recursive texture

   geoGL::Light3D          light0;              // Standard light
   geoGL::Camera3D         cam;

   // CDemoeffect, timing
   CEnvInfo               &m_oEnvInfo;
   unsigned int            nTimeStart, nElapsedTime;

   // Textures
   geoGL::TextureID        texture_cube2, texture_recurse;

   // Members
   void  LoadGLTextures();
   void  ReSizeGLScene(GLsizei width, GLsizei height);
   void  renderZany(GLenum);
   bool  initGLstuff();
   bool  initObjects();
	void  renderFrameInternal();
   GLuint loadGLTexture(md::CmediaDuke &, char *filename, char *textureName=NULL);
};

#endif // __CCubesACM_h__