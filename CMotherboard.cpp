//////////////////////////////////////////////////////////////////////////////
//
// CMotherboard.cpp
//	Implementation file for CMotherboard, CWhisp classes
//
//	-	CMotherboard defines a 3D fly-through of a motherboard
//	-	CWhip is a whisp of light used in the motherboard scene
//	-	Tabs set a 3.  (Tronster prefers real tabs, Moby Disk prefers spaces.)
//
//////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)
#include <stdlib.h>                             // Header has rand()
#include "CMotherboard.h"
#include "loaders3d.h"                          // GeoGL: 3SO file loader
#include "tesselate3d.h"                        // GeoGL: Object tesselator

// Define in CMotherboard header for manual control
#ifdef MANUAL_CONTROL
#include <iostream>                             // Manual control supports console output
#endif MANUAL_CONTROL

// Import geoGL and STL namespaces
using namespace geoGL;
using namespace std;

// User interface control settings
#define MOTIONSPEED  0.5f                       // Speed of manual control

// Nifty variations on appearance
#define FLUXINESS       0.75f                   // How much the glowing dots waver around
#define WHISP_SIZE      (5.0f/3.0f)             // Size of whispy lights
#define SOCKET_XSIZE    15.0f                   // Size of CPU socket
#define SOCKET_YSIZE    15.0f
#define LIGHT_HOVER_Z   10                      // Height of lighting light ball above motherboard

// RGB colors defining "background" of mindware microchip logo
#define BACK_GRAY    128
#define BACK_RED     128
#define BACK_GREEN   128
#define BACK_BLUE    128

// Light map settings
#define LIGHT_FWD_DIST  25
#define LIGHT_PWR2      6
#define LIGHT_DIM       (1<<LIGHT_PWR2)
#define LIGHT_SIZE      10.0f
#define PHONG           64

// Times in MS for start and duration of events
#define GLOWSTART    65000                         // Glowing starts
#define GLOWTIME     4000
#define CHIPSTART    (GLOWSTART+5000)              // Chip starts falling
#define CHIPTIME     5000
#define ANIMSTART    (CHIPSTART+CHIPTIME+GLOWTIME) // Chip logo animates
#define ANIMTIME     5000
#define FADESTART    (ANIMSTART+5000)              // Fadeout
#define FADETIME     3000
#define FINALSTART   (FADESTART+FADETIME+3000)
#define FINALTIME    1500

/******************************** Non-members *********************************/

// Random floating point number 0..1
static inline float frand01() { return rand() / static_cast<float>(RAND_MAX); }
// Random floating point number -1..1
static inline float frand11() { return (rand()/ static_cast<float>(RAND_MAX)) * 2 - 1; }

// Generic min/max routines
template <class T> static inline T TMAX(T T1, T T2) { return (T1>T2 ? T1 : T2); }
template <class T> static inline T TMIN(T T1, T T2) { return (T1<T2 ? T1 : T2); }

//////////////////////////////////////////////////////////////////////////////
//
// Compute animation factor 0..1
//    - Uses the current & start times and durations to compute a number 0..1
//	   
//
//	RETURNS: 0   = start of effect or before effect
//          0-1 = lienar progression between start/end time
//          1   = end of effect or after effect
//
//////////////////////////////////////////////////////////////////////////////
static float calcFactor(unsigned int nElapsedTime, unsigned int nStartTime, unsigned int nDuration)
{
   if (nElapsedTime<nStartTime)              return 0;
   if (nElapsedTime>nStartTime+nDuration)    return 1;
   return static_cast<float>(nElapsedTime-nStartTime) / nDuration;
}

//////////////////////////////////////////////////////////////////////////////
//
// Load a bitmap texture
//    - Load image using mediaduke (PNG/BMP/whatever)
//    - Create mipmapped texture
//    - Add to geoGL texture list (so object loader can access them)
//
// ARGS:    mediaDuke,     Mediaduke object for image loading
//          filename,      name+ext of image file to load
//          textureName,   unique string name of texture for use by object loader
//
//	RETURNS: texture ID number
//
//////////////////////////////////////////////////////////////////////////////
GLuint CMotherboard::loadGLTexture(
      md::CmediaDuke &mediaDuke, char *filename, char *textureName/*=NULL*/)
{
   GLuint      nTexture;                  // Texture ID number
   md::Cimage  textureImage;              // Mediaduke image

   // Load image, return on failure
   if (!mediaDuke.read(filename,textureImage))
      throwMessage("Unable to read %s",filename);

   // Cimage create an opengl texture - returns -1 on error
	nTexture = textureImage.makeGLTexture(m_oEnvInfo.glWantMipmap, m_oEnvInfo.glWantLinear);

   // Unable to create texture?  Throw an error
   if (nTexture<=0)
      throwMessage("Unable to create textures %s",filename);

   // Add texture to map using unique name.  GeoGL object loader can now use this texture.
   if (textureName)
      mapTextures[string(textureName)] = nTexture;

   // Return texture id
   return nTexture;
}

/******************************** CDemoEffect *********************************/


//////////////////////////////////////////////////////////////////////////////
//
// Construct motherboard effect
//    - Store a reference to the environment information
//
//////////////////////////////////////////////////////////////////////////////
CMotherboard::CMotherboard(CEnvInfo *oEnvInfo) : m_oEnvInfo(*oEnvInfo) {}


//////////////////////////////////////////////////////////////////////////////
//
// Destroy the motherboard
//   - Has Tronster's cool destructor tracking (for memory leaks)
//
//////////////////////////////////////////////////////////////////////////////
CMotherboard::~CMotherboard()
{ 
	unInit();
}


//////////////////////////////////////////////////////////////////////////////
//
// Start the motherboard effect
//    - Called by demo just before first frame
//    - Apply opengl settings that other effect may have overridden
//    - Begin time counter
//
//	RETURNS: true on success (always)
//
//////////////////////////////////////////////////////////////////////////////
bool CMotherboard::start()
{
   initGLstuff();             // Initialize opengl settings

   // Setup view frustum
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   // force 4/3 aspect ratio even if the window is stretched
	gluPerspective(45.0f, 4.0f/3.0f, 2,270.0f);
   frustum.init  (45.0f, 4.0f/3.0f, 2,270.0f);
   glMatrixMode(GL_MODELVIEW);

   // Save start time, and start the fly through
   nTimeStart = m_oEnvInfo.getMusicTime();
   flight.start(0);

   // Doesn't bother to check for errors, but it should
   return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// Stop the motherboard effect
//    - Clear any lighting and weird stuff so other effects are happy
//
//	RETURNS: true on success (always)
//
//////////////////////////////////////////////////////////////////////////////
bool CMotherboard::stop()
{
   // Disable light attenuation
   glLightf(GL_LIGHT0+light0.nLightID,GL_LINEAR_ATTENUATION,0);

   // Turn off all our lights
   light0.off();
   lightSocket.off();

   // Doesn't bother to check for errors, but it should
   return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// Initialize and load the motherboard
//    - Precompute quality values
//    - Load objects
//    - Initialize manual control if applicable
//
//	RETURNS: true on success (always)
//
//////////////////////////////////////////////////////////////////////////////
bool CMotherboard::init()
{
   // Grab demo quality and scale things according to it
   // It can range 0..10..infinity
   int nQuality = m_oEnvInfo.nDemoQuality;

   // Number of whisps to display at each quality setting (0..10)
   static const int num_Whispy[11] = {32,66,100,134,166,200,280,400,530,660,800};
   if (nQuality<11)
      nWhispy = num_Whispy[nQuality];        // Normal range (0..10) lookup in table
   else
      nWhispy = num_Whispy[10]*nQuality/10;  // Above 10, scale linearly beyond...

   // How much to tesselate objects
   //   - The BIGGER this is the LESS tesselated
   //   - The SMALLER this is, the MORE tesselated
   //   - This factor will actually refer to the length of the longest side in a
   //       tesselated object.  This results in a very nonlinear growth in # of sides!
   if (nQuality<=2)
      fTesselationFactor = 1000;             // Insanely large, never tesselate
   else
      fTesselationFactor = 5.0f/nQuality;    // Use quality to determine tesselation

   // Determine bump map to use
   // <=5 -- no bump map
   // 6,7 -- 256x256 bump map
   // 8+  -- 512x512 bump map
   if (nQuality>=8) m_szBumpFile = "mb_bump512.png"; else
   if (nQuality>=6) m_szBumpFile = "mb_bump256.png"; else
                    m_szBumpFile = NULL;
   

   // Initialize objects, textures, camera...
   initObjects();

   // Initialize the light map tables for bump map (if doing bump map)
   if (m_szBumpFile)
      initLight();

   // If under manual control, set the start position
   #ifdef MANUAL_CONTROL
   motion  = zero3D;
   rotation= zero3D;
   #endif

   // Doesn't bother to check for errors, but it should
   return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// Uninitialize
//    - Unload objects, textures, etc.  Generally free memory.
//
//	RETURNS: true on success (always)
//
//////////////////////////////////////////////////////////////////////////////
// ???WHG??? Add methods to cleanup geometry3d classes
bool CMotherboard::unInit()
{
   // Doesn't bother to check for errors, but it should
   return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// Load all textures
//    - Load PNG files using mediaduke object (available through CEnvInfo)
//    - Create procedural textures (for animating mindware logo)
//
//////////////////////////////////////////////////////////////////////////////
void CMotherboard::loadGLTextures()
{
   // Load generic PNG textures in the "smart" texture IDs
   text_cap32        = loadGLTexture(m_oEnvInfo.oMedia,"cap32.png", "cap32");
   text_res32        = loadGLTexture(m_oEnvInfo.oMedia,"res32.png", "res32");
   text_circuit      = loadGLTexture(m_oEnvInfo.oMedia,"chip2.png", "circuit");
   text_flare1       = loadGLTexture(m_oEnvInfo.oMedia,"flare1.png", "flare1");
   text_flare3       = loadGLTexture(m_oEnvInfo.oMedia,"flare3.png", "flare3");
   text_mbmap        = loadGLTexture(m_oEnvInfo.oMedia,"mb_map.png", "mb_map");
   text_socket       = loadGLTexture(m_oEnvInfo.oMedia,"socket.png", "socket");
   text_slot         = loadGLTexture(m_oEnvInfo.oMedia,"slot.png",   "slot");
   text_metal1       = loadGLTexture(m_oEnvInfo.oMedia,"metal.png",  "metal1");
   text_microchip1   = loadGLTexture(m_oEnvInfo.oMedia,"microchip1.png", "microchip1");
   text_microchip2   = loadGLTexture(m_oEnvInfo.oMedia,"mindwarethreesomecoma2.png","microchip2");
   text_greenchip    = loadGLTexture(m_oEnvInfo.oMedia,"creditschip.png","greenchip");

   //////////////// Custom mindware procedural texture ////////////////
   // This is a texture that starts out blank, but will get drawn on as the effect
   // progresses.  So we load the final image (img_mindware) and hold on to it.  Then 
   // another image of matching dimensions is created.  This image is made into a texture
   // that will be used on the actual object.
   
   // Load mindware logo 
   if (!m_oEnvInfo.oMedia.read("mindware.png",img_mindware))
      throwMessage("Unable to read %s","mindware.png");

   // Convert to RGB
   //   - This would be done automatically if we created a texture out of it
   if (img_mindware.palette)
      img_mindware.makeDataRGB();

   // Create RGB image to draw on, then fill with neutral color
	img_mindware0.create(img_mindware.x,img_mindware.y,3);
   memset(img_mindware0.data, BACK_GRAY, img_mindware0.x*img_mindware0.y*3);

   // Make image into a texture - throw error if problem arises
   text_mindware = img_mindware0.makeGLTexture(m_oEnvInfo.glWantLinear, m_oEnvInfo.glWantLinear);

   if (text_mindware<=0)
      throwMessage("Unable to create procedural texture mindware0");

   ///////////////////// Custom mindware lightmap /////////////////////
   // Load motherboard bump map -- if we are using one
   if (m_szBumpFile)
   {
      if (!m_oEnvInfo.oMedia.read(m_szBumpFile,img_bump))
         throwMessage("Unable to read %s",m_szBumpFile);

      // Verify bump map is 8-bit
      if (img_bump.bytesPerPixel != 1)
         throwMessage("Bump map should be 256 color grayscale!");

      // Create RGB image to draw on, then fill with neutral color
	   img_lightMap.create(img_bump.x,img_bump.y,3);
      memset(img_lightMap.data, 0, img_lightMap.x*img_lightMap.y*3);

      // Make image into a texture - throw error if problem arises
      text_lightMap = img_lightMap.makeGLTexture(m_oEnvInfo.glWantLinear, m_oEnvInfo.glWantLinear);

      if (text_lightMap<=0)
         throwMessage("Unable to create procedural texture text_lightMap");
   } else
      text_lightMap = 0; // Done bump map

   //////////////// Light beam 1D procedural texture ////////////////
   // And you thought 1D textures were useless...
   // Then light beam does not strike on the background color - only on texels
   // that are going to change.  To do this, the light beam uses a 1D texture
   // as a "mask" for where to draw a line, and where not to draw it

   // Create a "1D" image 
   // The width is same as the logo, but the height is 1
	img_animate.create(img_mindware.x,1,3);
   for (int zz=0; zz<img_animate.x; zz++) {
      img_animate.data[zz*3+0] = zz;
      img_animate.data[zz*3+1] = 0;
      img_animate.data[zz*3+2] = zz;
   }

   // Allocate Texture ID using GeoGL "smart" texture object
   text_animate.create();

   // Create a 1D texture 
   // -  MediaDuke doesn't have a nice function to do this for me

   // Bind 1D texture
   glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_1D, text_animate);

   // Use linear interpolation, mipmaps would be silly
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,m_oEnvInfo.glWantLinear);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,m_oEnvInfo.glWantLinear);

   // Create the texture
   glTexImage1D(GL_TEXTURE_1D,0,3,img_animate.x,0,
                     GL_RGB,GL_UNSIGNED_BYTE,img_animate.data);

   // Release the 1D texture binding 
	glBindTexture(GL_TEXTURE_1D, 0);
}

//////////////////////////////////////////////////////////////////////////////
//
// Create circular light
//    - Computes simple light into img_light image
//
//////////////////////////////////////////////////////////////////////////////
void CMotherboard::initLight()
{
   // Create a circular light
   img_light.create(LIGHT_DIM,LIGHT_DIM,1);
   for (int j=0; j<LIGHT_DIM; j++)
   {
      for (int i=0; i<LIGHT_DIM; i++)
      {
         float dist = (LIGHT_DIM/2-i)*(LIGHT_DIM/2-i) + (LIGHT_DIM/2-j)*(LIGHT_DIM/2-j);
         if (fabsf(dist)>1)
            dist = sqrtf(dist);
         int c = (int)(LIGHT_SIZE*dist);  //???WHG Random deviation + (rand()%7)-3;
         if (c<0)    c = 0;
         if (c>255)  c = 255;
         img_light.data[(j<<LIGHT_PWR2)+i] = 255-c;
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
//
// Set light attenuation
//    - Some sucky opengl drivers don't support this.  Maybe we need an option
//      to disable it.
//
//////////////////////////////////////////////////////////////////////////////
void CMotherboard::setAttenuation()
{
   // For those not versed in light attenuation:
   //   Attenuation causes light to affect distant objects less than near ones
   //   It can be constant, linear, or quadratic.
   //   Just a slight linear falloff allows a bit of realizm and cloaks
   //   far away details(and artifacts) until nearby

   // Maybe GeoGL's Light3D class should support this
   glLightf(GL_LIGHT0+light0.nLightID,GL_LINEAR_ATTENUATION,0.05f);
}

//////////////////////////////////////////////////////////////////////////////
//
// Apply opengl settings, fog, attenuation, light...
//    - Use glEnable to setup things that other demo effects may have changed
//    - Setup the lights and the ambient light
//
//////////////////////////////////////////////////////////////////////////////
void CMotherboard::initGLstuff()										// All Setup For OpenGL Goes Here
{
// OpenGL setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
   glEnable(GL_CULL_FACE);                      // Cull back faces
   glDisable(GL_NORMALIZE);                     // GeoGL creates unit normals at load
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glDepthFunc(GL_LEQUAL);								// Allows blends to be drawn over objects
   glEnable(GL_LIGHTING);                       // Lighting is key...

   setAttenuation();                            // Set light attenuation

// Light setup

   // Disable any lights left around by other effects
   for (int i=0; i<1; i++)
      glDisable(GL_LIGHT0+i);

   // No scene ambient light - the individual lights handle this
   Light3D::setSceneAmbient(fRGBA(0,0,0,0));

   // Light at viewer is on at full white
   light0.setLight(1.0f,1.0f,1.0f,1);
   light0.position(0.0f,0.0f,0.0f);
   light0.on();

   // Light at CPU socket comes later
   lightSocket.off();
}

//////////////////////////////////////////////////////////////////////////////
//
// Initialize openGL objects
//    - Load objects with GeoGL object loader
//    - Customize texture, light, position, transparency
//
//////////////////////////////////////////////////////////////////////////////
void CMotherboard::initObjects()
{
   // Load fly through path
   flight.Load("data.md/flythrough.dat");

   // Load all textures before objects
	loadGLTextures();

   // ABOUT GEOGL OBJECT LOADING
   // - Objects are loaded at the origin.  The motherboard is on the XY plane, so
   //   other objects are placed above it (+z) at various positions
   // - Objects may be tesselated so the triangle-mesh is more detailed.  This can
   //   aid lighting effects (particularly specular light) but eats memory+T&L time
   // - When objects are loaded, GeoGL will use the texture and light settings
   // specified in the 3SO file.  Some objects are "generic" (cube, plane) and
   // need to have the texture, color, etc. set manually.  Custom objects load
   // exactly as needed (resistor, CPU chip)

   // COMPUTER CASE: load, scale, position
   // - It is a 2-sided cube (2-sided else inside is culled)
   // - Metal texture is specified in the 3SO file
   computer = Load3SO("data.md/cube2s.3so",NULL,false);
   computer.rescale(80,120,40);
   computer.compile();

   // MOTHERBOARD: load, tesselate, scale, position
   // - Tesselated since lighting must be computed at more just the 4 corners!
   // - Uses large, detailed 512x512 texture
   motherboard = Load3SO("data.md/mb_map.3so",NULL,false);   
   motherboard = splitObj(motherboard,0.5f);
   motherboard.rescale(70,80,1);
   motherboard.position(0,-20,-37);
   motherboard.compile();

   // CPU SOCKET: load, scale, position
   // - Uses simple texture showing holes in socket
   socket = Load3SO("data.md/socket.3so",NULL,false);
   socket.rescale(SOCKET_XSIZE,SOCKET_YSIZE,0.8);
   socket.position = motherboard.position + fVector3D(0,49,0.8);
   socket.compile();

   // SOCKET HANDLE: load, scale, position, orient, 
   // - Reorient with the motherboard
   socketHandle = Load3SO("data.md/handle.3so",NULL,false);
   socketHandle.rescale(1,0.5f,36.5f);
   socketHandle.position = motherboard.position + fVector3D(21.5f,33.5f,0);
   socketHandle.direction(0,-90,0);
   socketHandle.compile();

   // CPU CHIP: load, scale, position, set texture
   // - Created from a generic cube so the texture must be set
   // - Size is slightly smaller than the socket
   CPUchip = Load3SO("data.md/cube.3so",NULL,false);
   CPUchip.rescale(SOCKET_XSIZE - 0.2f,SOCKET_YSIZE - 0.2f,0.5);
   CPUchip.position = socket.position + fVector3D(0,0,100);
   FOROBJSET(CPUchip,chipobj)
      chipobj.nTextureID = text_mindware;
   ENDFOROBJSET
   CPUchip.compile();

   // COOLCHIP: load, tesselate, scale, position
   // - Tesselated for nice specular highlight
   // - Created from a generic cube so the texture must be set
   // - Play with the default lighting too
   // - This used to be a big green chip, now it shows the credits
   coolchip = Load3SO("data.md/cube.3so",NULL,false);
   coolchip = splitObj(coolchip,0.75f*fTesselationFactor);
   coolchip.rescale(11,10,0.5);
   coolchip.position = motherboard.position + fVector3D(7,13,0.6);
   FOROBJSET(coolchip,chipobj)
      chipobj.nTextureID = text_greenchip;
      chipobj.color(0.6f,1.0f,0.6f);
      chipobj.setLight(0.15f,1.0f,0,1.5f,10);
      chipobj.bSpecularBlend = true;            // Apply 2-pass specular blend
      chipobj.bDrawSmooth = true;
   ENDFOROBJSET
   coolchip.compile();

   // CAPACITOR TABLE
   // - This is the coordinates of all the capacitors on the motherboard
   fVector3D cap_pos[num_Cap] = {
      fVector3D(-45,50,0), fVector3D(-45,44,0), fVector3D(-45,38,0), 
      fVector3D(-45,32,0), fVector3D(-45,26,0),

      fVector3D(-39,-6,0), fVector3D(-38.5,-12,0),
      fVector3D(-35,-32,0), fVector3D(-6,-33,0),
      fVector3D(-12,-44,0), fVector3D(7,-53,0),
      fVector3D(30,-44,0), fVector3D(34,-13,0),
      fVector3D(30,68,0), fVector3D(21,69,0),
      fVector3D(16,68,0), fVector3D(3,68,0),
      fVector3D(-1.5,68.5,0), fVector3D(-9,68.2,0)
   };

   // - This is the orientation of all the capacitors on the motherboard
   fVector3D cap_dir[num_Cap] = {
      fVector3D(0,270,10), fVector3D(4,320,6), fVector3D(-1,130,0), 
      fVector3D(11,200,7),  fVector3D(0,110,0),
      fVector3D(20,360,5), fVector3D(-19,210,0),
      fVector3D(0,140,-8),   fVector3D(7,90,-13),
      fVector3D(-5,110,3),  fVector3D(12,120,0),
      fVector3D(15,60,-1),  fVector3D(0,70,0),
      fVector3D(-3,280,0), fVector3D(2,150,-7),
      fVector3D(13,40,4),  fVector3D(0,10,0),
      fVector3D(-16,80,0),  fVector3D(13,256,2)
   };

   // MAIN CAPACITOR: load, tesselate, scale, position, reorient
   // - Tesselated for specular highlight
   ObjectSet3D cap_main = Load3SO("data.md/cap.3so",NULL,false);
   cap_main = splitObj(cap_main,0.75f*fTesselationFactor);
   cap_main.rescale(2.0f,1.5f,2.0f);
   cap_main.direction(90,0,0);
   cap_main.position = motherboard.position + fVector3D(0,0,1.5f*1.5f);
   cap_main.objects[0].bSpecularBlend = true;

   // Loop through capacitor table and create a capacitor for each position
   // - Copies of the original capacitor object (share vertices)
   // - Position & orientation from the above tables
   // - Random colors, 50% are specular
   for (int nCap=0; nCap<num_Cap; nCap++) {
      capacitor[nCap] = cap_main;                  // Copy original
      capacitor[nCap].position +=cap_pos[nCap];    // Add to position, direction
      capacitor[nCap].direction+=cap_dir[nCap];

      // Every other one
      if (nCap % 2==0)
      {
         // Is given a random color change of about 50%
         Object3D &obj = capacitor[nCap].objects[0];
         obj.diffuse.r = obj.diffuse.r*(frand01()/2 + 0.5f);
         obj.diffuse.g = obj.diffuse.g*(frand01()/2 + 0.5f);
         obj.diffuse.b = obj.diffuse.b*(frand01()/2 + 0.5f);
      }
      // And every other one is specular
      cap_main.objects[0].bSpecularBlend = (nCap % 2==0);
      capacitor[nCap].compile();
   }

   // RESISTOR TABLE
   // - This is the coordinates of all the capacitors on the motherboard
   fVector3D res_pos[num_Res] = {
      fVector3D(-55,68,0), fVector3D(-55,65,0),
      fVector3D(-55,62,0), fVector3D(-55,59,0),
   };

   // - This is the orientation of all the capacitors on the motherboard
   fVector3D res_dir[num_Res] = {
      fVector3D(0,0,90), fVector3D(0,0,90),
      fVector3D(0,0,90), fVector3D(0,0,90),
   };

   // MAIN RESISTOR: load, scale, position
   // - Tesselated for specular highlight
   ObjectSet3D res_main = Load3SO("data.md/res.3so",NULL,false);
   res_main.position = motherboard.position + fVector3D(0,0,1.0f);
   res_main.compile();

   // Loop through resistor table and create a resistor for each position
   // - Copies of the original resistor object (share vertices)
   // - Position & orientation from the above tables
   for (int nRes=0; nRes<num_Res; nRes++) {
      resistor[nRes] = res_main;
      resistor[nRes].position +=res_pos[nRes];
      resistor[nRes].direction+=res_dir[nRes];
   }

   // MAIN PCI SLOT: load, tesselate, scale, position
   // - Created from a generic cube so the texture must be set
   PCI[0] = Load3SO("data.md/slot.3so",NULL,false);
   PCI[0] = splitObj(PCI[0],1*fTesselationFactor);
   PCI[0].rescale(28,2,2);
   PCI[0].position = motherboard.position + fVector3D(-13,-27,1);
   PCI[0].compile();

   // Loop creating PCI slots, next to each other
   // - Each idential
   for (int nPCI=1; nPCI<num_PCI; nPCI++) {
      PCI[nPCI] = PCI[0];
      PCI[nPCI].position+=fVector3D(0,-11*nPCI,0);
   }

   // AGP SLOT: load, tesselate, scale, position
   // - Uses PCI slot object, but is a lighter color
   AGP = Load3SO("data.md/slot.3so",NULL,false);
   AGP = splitObj(AGP,1*fTesselationFactor);
   AGP.rescale(24,2,2);
   AGP.position= PCI[0].position + fVector3D(13,12,0);
   FOROBJSET(AGP,obj)
      obj.color(1,0.85f,0.48f);
      obj.setLight(0.1,0.4,0,0,0);
   ENDFOROBJSET
   AGP.compile();

   // ISA SLOT : load, tesselate, scale, position
   // - Uses PCI slot object, but is a darker color
   ISA = Load3SO("data.md/slot.3so",NULL,false);
   ISA = splitObj(ISA,1*fTesselationFactor);
   ISA.rescale(45,2,2);
   ISA.position= PCI[num_PCI-1].position + fVector3D(7,-6,0);
   FOROBJSET(ISA,obj)
      obj.diffuse = PCI[0].objects[0].diffuse / 4;
   ENDFOROBJSET
   ISA.compile();

   // MAIN DIMM SLOT: load, tesselate, scale, position
   // - Uses PCI slot object, but is darker, thinner, shorter
   ObjectSet3D DIMM0 = Load3SO("data.md/slot.3so",NULL,false);
   DIMM0 = splitObj(DIMM0,1*fTesselationFactor);
   DIMM0.rescale(17,1,1);
   DIMM0.direction = fVector3D(0,0,90);
   DIMM0.position = motherboard.position+fVector3D(47,54,0.5f);
   FOROBJSET(DIMM0,obj)
      obj.diffuse = PCI[0].objects[0].diffuse / 4;
   ENDFOROBJSET
   DIMM0.compile();

   // DIMM SLOTS: create 3 dimm slots (each pair forms one memory slot)
   DIMM[0] = DIMM0;     DIMM[0].position += fVector3D( 0,  0,0);
   DIMM[1] = DIMM0;     DIMM[1].position += fVector3D( 0,-34,0);
   DIMM[2] = DIMM0;     DIMM[2].position += fVector3D(-3,  0,0);
   DIMM[3] = DIMM0;     DIMM[3].position += fVector3D(-3,-34,0);
   DIMM[4] = DIMM0;     DIMM[4].position += fVector3D(-6,  0,0);
   DIMM[5] = DIMM0;     DIMM[5].position += fVector3D(-6,-34,0);

   // RAM CHIP: load, tesselate, scale, position
   // - Created from a generic cube so the texture must be set
   RAM = Load3SO("data.md/cube.3so",NULL,false);
   RAM.rescale(0.4f,33,7);
   RAM.position = motherboard.position+fVector3D(47,37,3.5f);
   FOROBJSET(RAM,obj)
      obj.nTextureID = text_circuit;
   ENDFOROBJSET
   RAM.compile();

   // SOME MICROCHIPS: load, scale, position, orientation
   // - Created from a generic cube so the texture must be set
   microchip[0] = Load3SO("data.md/chip.3so",NULL,false);
   microchip[0].rescale(11,1,5);
   microchip[0].direction = fVector3D(90,90,180);
   microchip[0].position = motherboard.position + fVector3D(-47,-62,1);
   microchip[0].compile();

   microchip[1] = Load3SO("data.md/chip.3so",NULL,false);
   microchip[1].objects[0].nTextureID = text_microchip2;
   microchip[1].rescale(6,1,3.5f);
   microchip[1].position = motherboard.position + fVector3D(-48,-44,1);
   microchip[1].direction = fVector3D(90,0,180);
   microchip[1].compile();

   microchip[2] = Load3SO("data.md/chip.3so",NULL,false);
   microchip[2].rescale(5,0.75f,2.5f);
   microchip[2].position = motherboard.position + fVector3D(30,-5,0.75f);
   microchip[2].direction = fVector3D(90,0,180);
   microchip[2].compile();

   // LIGHT BEAM: load, scale, position, orientation
   // - This is the light beam that "etches" onto the mindware chip
   // - Created from a generic plane so many things are changed 
   lightBeam = Load3SO("data.md/plane.3so",NULL,false);
   lightBeam.rescale(20,20,20);
   lightBeam.direction(90,-45,0);
   lightBeam.position = socket.position + fVector3D(-15,-15,20);
   FOROBJSET(lightBeam,obj)
      obj.color = fwhite;
      obj.bTransparency = true;              // Oooh, transparency!
      obj.nTextureType = GL_TEXTURE_1D;      // And a 1D texture!
      obj.nTextureID = text_animate;         // A procedural one!
      obj.ambient(0,0,0,1);                  // These settings make 50% transparency
      obj.diffuse(0,0,0,0.5f);               // They are magic, so don't muck with them
      obj.specular(0,0,0,1);
      obj.emission(1,1,1,1);
   ENDFOROBJSET

   // EXPLODING LIGHT FLARE: load, scale, position
   // - This object explodes out from the socket when the glowing starts
   // - Created from a generic plane and heavily modified
   flare = Load3SO("data.md/plane.3so",NULL,false);
   flare.position = motherboard.position + fVector3D(0,49,0.8);
   FOROBJSET(flare,flareobj)
      flareobj.color(1,1,1,1);
      flareobj.bTransparency = true;
      flareobj.nTextureID = text_flare3;
      flareobj.ambient(0,0,0,1);
      flareobj.diffuse(0,0,0,0);
      flareobj.specular(0,0,0,1);
      flareobj.emission(1,1,1,1);
   ENDFOROBJSET

   motherboardBump = motherboard;
   FOROBJSET(motherboardBump,obj)
      obj.color(1,1,1,1);
      obj.bTransparency = true;
      obj.nTextureID = text_lightMap;
      obj.ambient(0,0,0,1);
      obj.diffuse(0,0,0,0.5f);
      obj.specular(0,0,0,1);
      obj.emission(1,1,1,1);
   ENDFOROBJSET

   // GLOWING COLUMN: load, scale, position, orientation
   // - Created from a generic cube so attributes are changed
   // - GlowingOld is the original cube object and scale
   // - Glowing is an object that will change shape and size
   glowing    = Load3SO("data.md/cube.3so",NULL,false);  // Load twice instead of copying
   glowingOld = Load3SO("data.md/cube.3so",NULL,false);  // So they don't share vertices
   glowing.rescale(14.9f,1.0f,14.9f);
   glowingOld.rescale(14.9f,1.0f,14.9f);
   glowing.direction(90,0,0);
   glowing.position = motherboard.position + fVector3D(0,49,1.8);
   // Transparent, no texture
   FOROBJSET(glowing,obj)
      obj.color(1,1,1,1);
      obj.ambient(0,0,0,0);
      obj.diffuse(0,0,0,0.0);
      obj.specular(0,0,0,0);
      obj.bSpecularBlend = false;
      obj.emission(1,1,1,1);
      obj.bTransparency = true;
      obj.nTextureID = 0;
   ENDFOROBJSET

   // MAIN WHISP: load, scale, position, orientation
   // - Created from a generic plane so attributes are changed
   ObjectSet3D whispy_main = Load3SO("data.md/plane.3so",NULL,false);
   whispy_main.position = motherboard.position + fVector3D(0,49,5000);
   whispy_main.direction(45,0,0);
   whispy_main.rescale(WHISP_SIZE,WHISP_SIZE,WHISP_SIZE);
   FOROBJSET(whispy_main,obj)
      obj.color(1,1,1,1);
      obj.bTransparency = true;
      obj.nTextureID = text_flare1;
      obj.ambient(0,0,0,1);
      obj.diffuse(0,0,0,0);
      obj.specular(0,0,0,1);
      obj.emission(1,1,1,1);
   ENDFOROBJSET

   // Create a multitude of whisp objects that are waay to high to be seen
   // - since they are so high above the chip, they will get reset right away
   // - Scale the # of whisps according to the quality factor
   whispy.refnew(nWhispy);
   whispyData.refnew(nWhispy);
   for (int nWhisp=0; nWhisp<nWhispy; nWhisp++)
   {
      whispy[nWhisp] = whispy_main;          // Copy original, share vertices
      whispyData[nWhisp].basePos.z = 5000;   // Position way up high
   }

   // All objects are now ready to be rendered
}

//////////////////////////////////////////////////////////////////////////////
//
// Call a visible object
//    - Checks to see if the object is in the view frustum
//    - Only draws the object if it is 
//    - Call only works on compiled objects
//
// ARGS:    obj,           GeoGL "ObjectSet3D" to call
//
//	RETURNS: true if object visible, false if not
//////////////////////////////////////////////////////////////////////////////
bool CMotherboard::drawVisibleObject(ObjectSet3D &obj)
{
   if (frustum.visible(obj))
   {
      obj.call();
      return true;
   } else
      return false;
}

void CMotherboard::drawBump(int lx1, int ly1)
{
   int i, j, px, py, x, y, offs, c;
   int bump_w = img_lightMap.x;
   int bump_h = img_lightMap.y;

   offs = bump_w;
   int lx = -lx1 + LIGHT_DIM/2;   // lx1 ranges 0..LIGHT_DIM.  Change to -LIGHT_DIM/2 to +LIGHT_DIM/2
   int ly = -ly1 + LIGHT_DIM/2;   // ly1 ranges 0..LIGHT_DIM.  Change to -LIGHT_DIM/2 to +LIGHT_DIM/2
   for (j=1; j<bump_h; j++)
   {
      img_lightMap.data[offs*3 + 0] = 0;
      img_lightMap.data[offs*3 + 1] = 0;
      img_lightMap.data[offs*3 + 2] = 0;
      offs++;
      for (i=1; i<bump_w; i++)
      {
         // Calculate the slope for each pixels
         px = i + img_bump.data[offs-1] - img_bump.data[offs];
         py = j + img_bump.data[offs-bump_w] - img_bump.data[offs];

         // Lookup the first light map to have to light intensisy of this point
         x = px + lx;
         y = py + ly;
         if ((y>=0) && (y<LIGHT_DIM) && (x>=0) && (x<LIGHT_DIM))
            c = img_light.data[(y<<LIGHT_PWR2)+x];
         else
            c = 0;

         // Do not underflow or overflow
         if (c<0) c = 0;
         if (c>255) c = 255;

         img_lightMap.data[offs*3 + 0 ] = c;
         img_lightMap.data[offs*3 + 1 ] = c;
         img_lightMap.data[offs*3 + 2 ] = c;

         offs++;
      }
   }

   // Only update changed portions of the light map image
   // This could be MUCH more optimal
   int top, bott, subOffset;

   top = TMAX<int>(ly1 - LIGHT_DIM/2,0);                 // Top of area affected by light
   bott= TMIN<int>(ly1 + LIGHT_DIM/2,img_lightMap.y-1);  // Bottom of area affected by light
   subOffset = top*img_lightMap.x*3;                     // Offset of top

   // Update the light map - only update the portion that is affected
   // ???WHG 1) This still updates all the way across
   // ???WHG 2) This should leave trails since we did not include the area from the previous frame!
	glBindTexture(GL_TEXTURE_2D, text_lightMap);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, top, img_lightMap.x,bott-top+1, 
                       GL_RGB, GL_UNSIGNED_BYTE,img_lightMap.data+subOffset);
	glBindTexture(GL_TEXTURE_2D, 0);

}

//////////////////////////////////////////////////////////////////////////////
//
// Advance the light map effect
//    - This computes a bump map for the motherboard
//    - It is centered around the light in front of the viewer
//
//////////////////////////////////////////////////////////////////////////////
#define MB_X0  (-70 - 0)
#define MB_X1  (+70 - 0)
#define MB_Y0  (-80 - 20)
#define MB_Y1  (+80 - 20)
#define MB_X   (MB_X1 - MB_X0)
#define MB_Y   (MB_Y1 - MB_Y0)

void CMotherboard::advanceLightMap()
{
   // Exit if we are not doing bump mapping
   if (!m_szBumpFile)
      return;

   int lx,ly;
   fVector2D lightPos;

   // Compute position of light, projected onto MB, range 0..1
   lightPos.x = (light0.position.x - MB_X0) / MB_X;
   lightPos.y = (light0.position.y - MB_Y0) / MB_Y;

   // Clamp in case it goes off the MB
   if (lightPos.x<0) lightPos.x = 0; else
   if (lightPos.x>1) lightPos.x = 1;
   if (lightPos.y<0) lightPos.y = 0; else
   if (lightPos.y>1) lightPos.y = 1;

   // Scale it to the resolution of the light map
   lx = static_cast<int>(lightPos.x * img_lightMap.x);
   ly = static_cast<int>(lightPos.y * img_lightMap.y);

   // Bump map not changed, return
   if (lx==lx0 && ly==ly0)
      return;
   lx0 = lx;
   ly0 = ly;

   drawBump(lx,ly);
}

//////////////////////////////////////////////////////////////////////////////
//
// Advance the animation for the mindware chip
//    - Computes the procedural texture img_mindware and sends it to opengl
//    - Computes the procedural 1D texture for the "etching" light beam
//
// ARGS:    fAnimateFactor,   0..1 (0 is starting, 1 is completed)
//////////////////////////////////////////////////////////////////////////////
void CMotherboard::advanceChipTexture(float fAnimateFactor)
{
   static const int nAhead     = 50;                // # of pixels ahead for white glow
   static const int nWhiteness = 5;                 // Brightness factor for these pixels

   // The image appears on the chip coming from one corner moving to another.
   // This is commonly called a "diagonal wipe"
   // Just ahead of the wipe, the pixels-to-be glow white, and fade down to the
   // proper color.  This only happens on non-background pixels.
   //
   // In addition, a glowing beam carves out the sections that are being drawn.
   // The beam fires at the glowing points that are ahead of the wiped pixels.
   //   - The wipe cannot depend on previous frames (due to frameskip)
   //   - A 1D texture must be plotted with the section the beam carves
   //   - The texture must be NxN, RGB8
   //   - The 1D texture must be Nx1
   //   - "Diagonals" range 0..2N
   //   - The diagonal of a pixel at (x,y) in an NxN texture is (x+y)

   md::Cimage &image0 = img_mindware0;        // Working image to draw on
   md::Cimage &image  = img_mindware;         // Final image to draw from

   int nCurrDiag = static_cast<int>(
                     fAnimateFactor * (image.x+image.y));// Current diagonal at full color
   int nGlowDiag = nCurrDiag + nAhead;                   // Current glowing diagonal
   int nOffset   = 0;                                    // Offset of (x,y) pixel in texture

   // Erase the carved 1D texture.  This is filled in as we go
   memset(img_animate.data,0,img_animate.x*3);

   // Loop through texture
   for (int y=0; y<image.y; y++)
   {
      for (int x=0; x<image.x; x++)
      {
         int nDiag     = x+y;                      // Current diagonal (0..x+y)

         // Is this diagonal on or before the current diagonal?
         if (nDiag <= nCurrDiag)
         {
            // Then set the color to the final color
            image0.data[nOffset+0] = image.data[nOffset+0];
            image0.data[nOffset+1] = image.data[nOffset+1];
            image0.data[nOffset+2] = image.data[nOffset+2];
         } else

         // Is this diagonal between the current diagonal and the white glowing diagonal?
         if (nDiag <= nCurrDiag + nAhead)
         {
            // Only glow pixels that are not "background" gray
            if (image.data[nOffset+0]!=BACK_RED || 
                image.data[nOffset+1]!=BACK_GREEN ||
                image.data[nOffset+2]!=BACK_BLUE)
            {
               // Glow far ahead pixels white, fade down to proper color

               int nWhite = (nDiag - nCurrDiag) * nWhiteness;     // Whiteness factor

               // Add the whiteness to the pixel color
               image0.data[nOffset+0] = TMIN(image.data[nOffset+0] + nWhite,255);
               image0.data[nOffset+1] = TMIN(image.data[nOffset+1] + nWhite,255);
               image0.data[nOffset+2] = TMIN(image.data[nOffset+2] + nWhite,255);

               // Is this the brightest glowing diagonal?
               if (nDiag == nCurrDiag + nAhead)
               {
                  // Yes it is - this is where the 1D carving glow strikes
                  // This pixel is glowing, so we add it to the mask.  This is a mess.

                  int nLen, nVal;

                  // Need: 1) The length of the current diagonal (0..N)
                  //       2) The coordinate to decide the offset along that diagonal
                  if (nDiag <= img_animate.x)
                    { nLen = nDiag; nVal = x; }
                  else
                    { nLen = 2*img_animate.x - nDiag; nVal = img_animate.x - y; }

                  // Compute the offset (-N/2..N/2) along the diagonal
                  // Then compute the pixel in the 1D image that it corresponds to
                  int nOffset = nLen/2 - nVal;
                  int nPixel  = img_animate.x/2 - nOffset;

                  // I think it is going out of range by 1 sometimes, due to the rounding
                  if (nPixel>=0 && nPixel<img_animate.x)
                  {
                     // Compute offset, set to white
                     int z = nPixel * 3;
                     img_animate.data[z+0] = 255;
                     img_animate.data[z+1] = 255;
                     img_animate.data[z+2] = 255;
                  }
               } // end 1D texture at brightest diagonal
            } // end any pixel that is not the background color
         } // end which diagonal

         // Update offset
         nOffset += 3;

      } // end X
   } // end Y

   // Wow, now lets move the plane onto the appropriate diagonal
   // We compute the position of the texture's diagonal in 3D space
   float fGlowDiagonal = (float)(nCurrDiag+nAhead)/(image.x+image.y);
   lightBeam.position = 
      socket.position + 
      fVector3D(-SOCKET_XSIZE+SOCKET_XSIZE*2*fGlowDiagonal,
                -SOCKET_YSIZE+SOCKET_YSIZE*2*fGlowDiagonal, 20);

   // Update the chip drawing
   // Use glTexSubImage2D to update a texture
	glBindTexture(GL_TEXTURE_2D, text_mindware);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img_mindware0.x,img_mindware0.y, 
                       GL_RGB, GL_UNSIGNED_BYTE,img_mindware0.data);
	glBindTexture(GL_TEXTURE_2D, 0);

   // Update the light beam
   // Use glTexSubImage1D to update a texture
	glBindTexture(GL_TEXTURE_1D, text_animate);
   glTexSubImage1D(GL_TEXTURE_1D, 0, 0, img_animate.x, 
                       GL_RGB, GL_UNSIGNED_BYTE,img_animate.data);
	glBindTexture(GL_TEXTURE_1D, 0);

}

//////////////////////////////////////////////////////////////////////////////
//
// Render the current frame
//    - Draws everything as computed in last advanceFrame, updates nothing
//
// RETURN:  true on success (always)
//////////////////////////////////////////////////////////////////////////////
bool CMotherboard::renderFrame()
{
   // These are used as backups when fading the light out
   fRGBA l0Ambient,l0Diffuse,l0Specular; int l0Shininess;
   fRGBA lsAmbient,lsDiffuse,lsSpecular; int lsShininess;

   // Camera
	glMatrixMode(GL_MODELVIEW);         // Reset modelview matrix
	glLoadIdentity();
   flight.cam.execute();               // Apply camera transform
   frustum.setFrustum(flight.cam);     // Rotate view frustum according to camera

   // If we are in fadeout mode, fade the lights to a lower level
   if (nElapsedTime>=FADESTART) {
      // Calculate fade factor from 0..1
      float fFadeFactor = calcFactor(nElapsedTime,FADESTART,FADETIME);

      // Save light settings
      l0Ambient = light0.ambient;      lsAmbient = lightSocket.ambient;
      l0Diffuse = light0.diffuse;      lsDiffuse = lightSocket.diffuse;
      l0Specular= light0.specular;     lsSpecular= lightSocket.specular;
      l0Shininess=light0.shininess;    lsShininess=lightSocket.shininess;

      // Fade out scaled light settings according to fade factor computed above
      fFadeFactor = 1-fFadeFactor;
      light0.ambient = l0Ambient*fFadeFactor;
      light0.diffuse = l0Diffuse*fFadeFactor;
      light0.specular= l0Specular*fFadeFactor;
      lightSocket.ambient = lsAmbient*fFadeFactor;
      lightSocket.diffuse = lsDiffuse*fFadeFactor;
      lightSocket.specular= lsSpecular*fFadeFactor;

      // If we are in the final fadeout, fade the saved lights too, not just the copies
      if (nElapsedTime>=FINALSTART)
      {
         float fRate = (1.0f/30.0f) * (FINALTIME / 1000.0f);      // Speed of final fade down
         l0Ambient  -= fwhite*fRate*m_oEnvInfo.fFrameFactor;    lsAmbient  -= fwhite*fRate*m_oEnvInfo.fFrameFactor;
         l0Diffuse  -= fwhite*fRate*m_oEnvInfo.fFrameFactor;    lsDiffuse  -= fwhite*fRate*m_oEnvInfo.fFrameFactor;
         l0Specular -= fwhite*fRate*m_oEnvInfo.fFrameFactor;    lsSpecular -= fwhite*fRate*m_oEnvInfo.fFrameFactor;
         l0Shininess-= fRate*m_oEnvInfo.fFrameFactor;           lsShininess-= fRate*m_oEnvInfo.fFrameFactor;
      }
   }
   ////////// BEGIN STUFF THAT WILL BE AFFECTED BY THE FIRST FADEOUT //////////

   // Apply light position & color
   light0.executeLight();
   lightSocket.executeLight();

   // Draw static objects
   drawVisibleObject(computer);              // Computer case            
   drawVisibleObject(motherboard);           // Motherboard plane            
   drawVisibleObject(coolchip);              // Big chip near CPU socket (credits on it)
   drawVisibleObject(RAM);                   // Memory chip
   drawVisibleObject(AGP);                   // AGP, ISA slots
   drawVisibleObject(ISA);

   // Multiple static objects
   int nObject;
   for (nObject=0; nObject<num_Res; nObject++)  drawVisibleObject(resistor[nObject]);
   for (nObject=0; nObject<num_Cap; nObject++)  drawVisibleObject(capacitor[nObject]);
   for (nObject=0; nObject<num_PCI; nObject++)  drawVisibleObject(PCI[nObject]);
   for (nObject=0; nObject<num_DIMM; nObject++) drawVisibleObject(DIMM[nObject]);
   for (nObject=0; nObject<num_Chip; nObject++) drawVisibleObject(microchip[nObject]);

   /////////// END STUFF THAT WILL BE AFFECTED BY THE FIRST FADEOUT ////////////

   // Okay, now there are a few things that we want drawn even if the light is out
   // So restore the light
   if (nElapsedTime>=FADESTART) {
      // Bring back light from saved values
      light0.ambient = l0Ambient;
      light0.diffuse = l0Diffuse;
      light0.specular= l0Specular;
      lightSocket.ambient = lsAmbient;
      lightSocket.diffuse = lsDiffuse;
      lightSocket.specular= lsSpecular;

      // Re-apply the lights
      light0.executeLight();
      lightSocket.executeLight();
   }

   // Static objects not affected by the first fade out
   drawVisibleObject(socketHandle);          // Movable handle to CPU socket
   drawVisibleObject(socket);                // CPU socket            
   drawVisibleObject(CPUchip);               // CPU chip            

   // Transparent objects, effects
   // - Disable writing to the depth buffer
   glDepthMask(GL_FALSE);

   // Draw flare of light that grows out from the CPU socket, along the motherboard
   flare.execute();

   // Draw light beam etching mindware logo onto CPU chip
   // - Only if the time is right
   if (nElapsedTime>ANIMSTART && nElapsedTime<ANIMSTART+ANIMTIME)
      lightBeam.execute();

   // Draw glowing column of light & the little whisps
   // - Only if the time is right
   if (nElapsedTime>GLOWSTART && nElapsedTime<CHIPSTART+CHIPTIME)
   {
      // Draw the column
      glowing.execute();
      // Draw the whisps
      for (nObject=0; nObject<nWhispy; nObject++)
         whispy[nObject].execute();
   }

   // Draw bump map on motherboard
   if (m_szBumpFile)
      motherboardBump.execute();

   /* Glowing sphere
      -- This is commented out by popular demand.  I really liked it because you could see
         what was causing the light.  But no matter what I did, it looked lame.  So it is
         removed.
	glPushMatrix();
      // No texture, enable additive blending
      glBindTexture(GL_TEXTURE_2D,0);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glTranslatef(light0.position.x,light0.position.y,light0.position.z);

      // White, vary 40%-60% transparency
      glMaterialfv(GL_FRONT,GL_AMBIENT,fRGBA(0,0,0,1));
      glMaterialfv(GL_FRONT,GL_DIFFUSE,fRGBA(0,0,0,frand01()*0.2f + 0.4f));
      glMaterialfv(GL_FRONT,GL_SPECULAR,fRGBA(0,0,0,1));
      glMaterialfv(GL_FRONT,GL_EMISSION,fRGBA(1,1,1,1));

      // Draw sphere
	   GLUquadricObj *qoSphere = gluNewQuadric();
		gluQuadricTexture(qoSphere, GL_FALSE);
		gluQuadricDrawStyle(qoSphere, GLU_FILL);
		gluSphere(qoSphere, 0.5f, 10, 10);
		gluDeleteQuadric(qoSphere);

      glDisable(GL_BLEND);
	glPopMatrix();
   */

   // Draw debugging info if in debug mode
   #ifdef _DEBUG
	if (m_oEnvInfo.bShowDebugInfo)
	{
      glMaterialfv(GL_FRONT,GL_EMISSION,fwhite);
      m_oEnvInfo.OglDebug.printf(0,48,0,"start: %d, frame: %d",nTimeStart,flight.getFrame());
      glMaterialfv(GL_FRONT,GL_EMISSION,fblack);
   }
   #endif

   // Re-enable the depth mask
   glDepthMask(GL_TRUE);

   // This is unneeded
   glFlush();

   return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// Advance the animation for the motherboard effect
//    - Computes the procedural texture img_mindware and sends it to opengl
//    - Computes the procedural 1D texture for the "etching" light beam
//
// RETURNS:    true for success (always)
//////////////////////////////////////////////////////////////////////////////
bool CMotherboard::advanceFrame()
{
   // Update elapsed time
   nElapsedTime = m_oEnvInfo.getMusicTime() - nTimeStart;

   // Apply motion set during manual control
#ifdef MANUAL_CONTROL
   flight.cam.position += motion;               // Apply movement, rotation vectors
   flight.cam.rotateDeg(rotation);
   motion  = motion/2;                          // Slow down motion, rotation
   rotation= rotation/2;
#else
   // Tell the fly-through the new time - it will update position accordingly
	flight.setCamera(nElapsedTime);
#endif

   // CPU chip falling
   if (nElapsedTime>=CHIPSTART)
   {
      // Compute factor 0..1, and new chip position
      float fChipFactor = calcFactor(nElapsedTime,CHIPSTART,CHIPTIME);
      CPUchip.position = socket.position + fVector3D(0,0,100*(1-fChipFactor)+1);
   }

   // Do glowing socket stuff
   // (do this after the chip falls because glowingBurst() references the chip position)
   if (nElapsedTime>=GLOWSTART && nElapsedTime<CHIPSTART+CHIPTIME) {
      // Glowing factor 0...1
      float fGlowFactor = calcFactor(nElapsedTime,GLOWSTART,GLOWTIME);

      // Drawing the burst of light
      advanceGlowingBurst(fGlowFactor);

      // Drawing the whisps of light
      for (int i=0; i<nWhispy; i++) {
         if (whispy[i].position.z >= CPUchip.position.z - WHISP_SIZE*2) {
            whispyData[i].reset();
            whispy[i].objects[0].emission(0.3f,frand01()+0.3f,frand01()+0.3f,1);
         }
         whispyData[i].advance(m_oEnvInfo.fFrameFactor);
         whispy[i].objects[0].diffuse(0,0,0,whispyData[i].bright);
         whispy[i].position = motherboard.position + fVector3D(0,49,0) + whispyData[i].fluxPos;
      }
   }

   // Socket handle returns after chip falls
   if (nElapsedTime>=CHIPSTART+CHIPTIME) {
      float fSockFactor = calcFactor(nElapsedTime,CHIPSTART+CHIPTIME,GLOWTIME);

      socketHandle.direction(0,fSockFactor*-90,0);
   }

   // Chip logo appears
   if (nElapsedTime>=ANIMSTART) {
      float fAnimFactor = calcFactor(nElapsedTime,ANIMSTART,ANIMTIME);

      advanceChipTexture(fAnimFactor);
   }

   // Light source is 10 units ahead of the camera position
   light0.position = flight.cam.position + flight.cam.getForward() * LIGHT_FWD_DIST;
   light0.position.z = motherboard.position.z + LIGHT_HOVER_Z;

   // Update light map
   advanceLightMap();
   return true;
};

//////////////////////////////////////////////////////////////////////////////
//
// Animate glowing from CPU socket
//   - Advance the column of light from the CPU socket
//   - Enlarge the flare from the CPU socket
//
// ARGS:    fGlowFactor,   0..1 (0 is starting, 1 is completed)
//////////////////////////////////////////////////////////////////////////////
void CMotherboard::advanceGlowingBurst(float fGlowFactor)
{
   Object3D &glowObj = glowing.objects[0];      // Working column of light
   Object3D &glowOld = glowingOld.objects[0];   // Original column of light

   // First half of flare is fast
   if (fGlowFactor<0.50f) {
      if (fGlowFactor>0.01f)                 // Don't scale below a certain size
         flare.scale(fGlowFactor*1000);      //  or it goofs up
      flare.objects[0].diffuse(0,0,0,fGlowFactor*2);
   } else
   // Second half of flare is really fast
   if (fGlowFactor<1.00f) {
      flare.scale(500 + (fGlowFactor-0.5f)*2500);
      flare.objects[0].diffuse(0,0,0,(1-fGlowFactor)*2);
   }
   // Rotate the flare for effect
   flare.direction = fVector3D(0,0,fGlowFactor*360);

   // Apply the light coming from the socket
   // - This yellowish light floats up from the motherboard to just above the socket
   // - It flickers randomly, and only affects very nearby objects (due to attenuation)
   lightSocket.on();
   lightSocket.setLight(fGlowFactor,fGlowFactor,fGlowFactor,64,fRGBA(1,1,0));
   lightSocket.position = motherboard.position + fVector3D(0,49,1 + fGlowFactor*5);
   glLightf(GL_LIGHT0+lightSocket.nLightID,GL_LINEAR_ATTENUATION,0.05f+frand01()*0.025f);

   // Turn the socket handle
   socketHandle.direction(0,fGlowFactor*90 - 90,0);

   // The glowing column of light has the alpha value randomly fluctuate by 10%
   // - It grows upward (z+) from the original position
   glowObj.diffuse(0,0,0,fGlowFactor/4+frand01()*0.1f);
   float fHeight = CPUchip.position.z - glowing.position.z - 1;
   glowObj.points[2].y = glowOld.points[2].y + fGlowFactor*fHeight;
   glowObj.points[3].y = glowOld.points[3].y + fGlowFactor*fHeight;
   glowObj.points[6].y = glowOld.points[6].y + fGlowFactor*fHeight;
   glowObj.points[7].y = glowOld.points[7].y + fGlowFactor*fHeight;
}

/********************************** CWhisp ***********************************/

//////////////////////////////////////////////////////////////////////////////
//
// Reset a whisp
//    - Move to a random position at the base of the socket
//    - Pick random speeds and motions
//////////////////////////////////////////////////////////////////////////////
void CWhisp::reset()
{
   // Place somewhere within the socket, directly on the motherboard
   basePos = fVector3D(frand11()*SOCKET_XSIZE, frand11()*SOCKET_YSIZE, 0);
   // Reset flux animation factor
   fluxCnt = 0;
   // Pick a vertical motion speed and a speed for fluxiness
   zSpeed  = frand01() * 1.0f + 0.4f;
   rSpeed  = frand01() * 0.3f + 0.1f;

   // Pick a direction 0 - 360 degrees for the particle to fluctuate
   float fluxDeg = rand() % 360;
   fluxDir(sinf(DtoR(fluxDeg)), cosf(DtoR(fluxDeg)), 0);
   // Brightness starts at fully transparent
   bright = 0.0f;
}

//////////////////////////////////////////////////////////////////////////////
//
// Advance the animation for a CWhisp
//    - Moves object position, updates brightness
//
// ARGS:    fFrameFactor,   0..1 (0 is starting, 1 is completed)
//////////////////////////////////////////////////////////////////////////////
void CWhisp::advance(float fFrameFactor)
{
   // Move whisp according to whispy speed and frame rate
   basePos.z += zSpeed * fFrameFactor;                // Move along z axis
   fluxCnt   += rSpeed * fFrameFactor;                // Swirl along xy axis

   // Final position is base position + fluxing position * fluxiness
   fluxPos = basePos + fluxDir * sinf(fluxCnt) * FLUXINESS;

   // Make brighter, unless at max brightness
   if (bright>=1)
      bright = 1;
   else
      bright += zSpeed * 0.05f * fFrameFactor;
}