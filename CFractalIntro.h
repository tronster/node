#ifndef __CFractalIntro_h__
#define __CFractalIntro_h__

#pragma warning(disable:4786)

#ifdef _DEBUG
#include "gldebug.h"
#endif

#include "CDemoEffect.h"
#include "geometry3d.h"
#include "flythru3d.h"
#include "mduke.h"
#include "tiler.h"
#include "CIFS.h"

class CFractalIntro : public CDemoEffect
{
public:
   // CDemoEffect
	CFractalIntro(CEnvInfo *);
  ~CFractalIntro();

   // vidGL virtual overrides
	bool	renderFrame();
	bool	advanceFrame();
   bool  init();
   bool  unInit();
   bool  start();
   bool  stop();

protected:
   // Lights, Camera
   geoGL::Light3D                light0;              // Standard light
   geoGL::Camera3D               cam;

   // Action
   refptr<geoGL::ObjectSet3D>    objCube;
   CTilerGL                      tileBG;
   md::Cimage                    oBigBackground;
   int                           nCenterTile;

   // Fractal tree
   CIFS                          tree1;
   refptr<geoGL::fVector2D>      pPointsGrow;
   float                         fCurrDepth;

   // Wind on tree
   float                         fWindX, fWindY;

   // Symbols
   refptr<geoGL::TextureID>      pSymbols;
   geoGL::ObjectSet3D            objSymbol;
   int                           nSymbol;
   float                         fSymFactor;

   // Timing, CDemoEffect
   CEnvInfo &                    m_oEnvInfo;
   unsigned int                  nTimeStart, nElapsedTime;
   float                         fLineScale;

   // CFractalIntro
   void  loadGLTextures();
   void  initGLstuff();
   void  initObjects();
   void  initTree();
   bool  renderSymbols();
   bool  renderTree();
   bool  renderBackground();
   bool  renderParticles();
   bool  advanceSymbols();
   bool  advanceTree();
   bool  advanceBackground();
   bool  advanceParticles();
   GLuint loadGLTexture(md::CmediaDuke &, char *filename, char *textureName=NULL);
};

#endif