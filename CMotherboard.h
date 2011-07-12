//////////////////////////////////////////////////////////////////////////////
//
// CMotherboard.h
//	Header file for CMotherboard, CWhisp classes
//
//	-	CMotherboard defines a 3D fly-through of a motherboard
//	-	CWhip is a whisp of light used in the motherboard scene
//	-	Tabs set a 3.  (Tronster prefers real tabs, Moby Disk prefers spaces.)
//
//////////////////////////////////////////////////////////////////////////////
#ifndef __CMotherboard_h__
#define __CMotherboard_h__

// Define this constant to allow keyboard control instead of auto fly-through
#undef MANUAL_CONTROL

// GLdebug library is used for outputting debug text to the screen
// - only used in debug mode
#ifdef _DEBUG
#include "gldebug.h"
#endif

// Header files
#include "geometry3d.h"             // GeoGL geometry and opengl classes
#include "flythru3d.h"              // GeoGL cubic interpolating camera
#include "frustum3d.h"              // GeoGL frustum class for clipping objects
#include "mduke.h"                  // Mediaduke loader
#include "CDemoEffect.h"            // DemoEffect base class for CMotherboard

// Various constants for arrays
#define num_PCI        5            // Number of PCI slots
#define num_Cap       19            // Number of capacitors
#define num_Res        4            // Number of resistors
#define num_DIMM       6            // Number of DIMM slits
#define num_Chip       3            // Number of generic microchips

//////////////////////////////////////////////////////////////////////////////
//
//	Whisp of light that flies upward in smooth arcs
//   - Moves along Z-axis, fluidly "fluxing" from side to side
//   - Starts out dark, gets brighter to fade in (instead of just appearing)
// 
//////////////////////////////////////////////////////////////////////////////
class CWhisp
{
public:
   geoGL::fVector3D  basePos,                // Base position of whisp, moves along z
                     fluxDir,                // Direction of flux on x-y plane
                     fluxPos;                // Current position including flux
   float             zSpeed,                 // Speed along z-axis
                     rSpeed;                 // Speed of fluxing
   float             fluxCnt,                // Flux animation variable
                     bright;                 // Brightness

   inline CWhisp()                           // Create an empty CWhisp
      { memset(this,0,sizeof(*this)); }
   void reset();                             // Reset CWhisp to starting values
   void advance(float fFrameFactor);         // Step animation of CWhisp
};

//////////////////////////////////////////////////////////////////////////////
//
//	Main motherboard scene
//   - Implements CDemoEffect interface (virtual base class)
//   - Contains objects, textures, lights, camera
//   - Demonstrates full functionality of GeoGL
// 
//////////////////////////////////////////////////////////////////////////////
class CMotherboard : public CDemoEffect
{
public:
//CDemoEffect
   // Constructor/destructor
	CMotherboard(CEnvInfo *);
  ~CMotherboard();

   // Overloads for initialization, animation
	bool  advanceFrame();
	bool  renderFrame();
   bool  init();
   bool  unInit();
   bool  start();
   bool  stop();

protected:
//CMotherboard
   // Initialization
   void  initGLstuff();                      // Apply opengl settings, fog, attenuation, light...
   void  initObjects();                      // Load objects, setup properties, positions...
   void  initLight();                        // Initialize light map & associated tables
   void  loadGLTextures();                   // Load textures, allocate procedural textures
   void  setAttenuation();                   // Set light attenuation values

   // Load/mipmap a single texture
   GLuint loadGLTexture(
      md::CmediaDuke &mediaDuke, 
      char *filename, 
      char *textureName=NULL);

   // Animation
   bool  drawVisibleObject(geoGL::ObjectSet3D &obj);  // Draw specified object if in view frustum
   void  advanceGlowingBurst(float fGlowFactor);      // Animate the glowing burst of light on chip
   void  advanceChipTexture (float fAnimateFactor);   // Animate procedural chip texture ("mindware logo")
   void  advanceLightMap();                           // Animate the light map for motherboard
   void  drawBump(int,int);                           // Draw bump map into light map

   // Standard objects
   geoGL::ObjectSet3D      computer, motherboard, socket, CPUchip,   // singular
                           coolchip, ISA, AGP, RAM, socketHandle;
   geoGL::ObjectSet3D      capacitor[num_Cap], PCI[num_PCI],         // multiple
                           resistor[num_Res],
                           DIMM[num_DIMM], microchip[num_Chip];

   // Special effect objects
   geoGL::ObjectSet3D         glowing, glowingOld;          // Glowing light column
   geoGL::ObjectSet3D         flare;                        // Exploding light flare
   geoGL::ObjectSet3D         lightBeam;                    // Light etching MW logo
   geoGL::ObjectSet3D         motherboardBump;

   // Light whisps from socket
   int                        nWhispy;                      // Number of them
   refptr<geoGL::ObjectSet3D> whispy;                       // Light whisps from socket
   refptr<CWhisp>             whispyData;                   //   and their data

   // Lights
   geoGL::Light3D             light0,                       // Standard light
                              lightSocket;                  // Light at CPU socket
   int                        lx0,ly0;                      // Position of light0 in prev frame

   // Fly-through
   geoGL::FlyThru3D           flight;                       // Flight path
   geoGL::Frustum3D           frustum;                      // View frustum

   // Start time, elapsed time since start
   unsigned int               nTimeStart, nElapsedTime;

   // CDemoEffect environment information (resolution, color depth, quality...)
   CEnvInfo &                 m_oEnvInfo;
   float                      fTesselationFactor;
   char *                     m_szBumpFile;

   // Textures
   geoGL::TextureID        text_cap32, text_res32, text_metal1;
   geoGL::TextureID        text_circuit,  text_flare1, text_flare3, text_mbmap;
   geoGL::TextureID        text_socket, text_slot, text_animate,
                           text_microchip1, text_microchip2, text_greenchip;
   geoGL::TextureID        text_mindware, text_lightMap;
   // Images
   md::Cimage              img_mindware,                    // Mindware logo (final)
                           img_mindware0,                   // Mindware logo (procedural)
                           img_animate,                     // Light beam "etching" logo
                           img_bump,
                           img_light,
                           img_lightMap;

   // User interface if enabled
   #ifdef MANUAL_CONTROL
   geoGL::fVector3D        motion, rotation;
   #endif
};

#endif