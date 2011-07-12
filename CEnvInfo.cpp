//////////////////////////////////////////////////////////////////////////////
//
// CEnvInfo.h
//	
//	Class to hold environment information
//
//////////////////////////////////////////////////////////////////////////////

#include "CEnvInfo.h"

CEnvInfo::CEnvInfo() : 
	unMusicTime(0), 
   nSaveToDisk(0),
   glWantMipmap(GL_LINEAR_MIPMAP_LINEAR), glWantLinear(GL_LINEAR)
{
}

//////////////////////////////////////////////////////////////////////////////
//
// Calculate GL quality settings
//    - Calculate the values for glWantMipmap and glWantLinear from the nDemoQuality
//
//	PRE:  nDemoQuality must be set
// POST: gl variables are setup
//
//////////////////////////////////////////////////////////////////////////////
void CEnvInfo::calcGLsettings()
{
   // Clamp demo quality below 0, but it can go above 10
   if (nDemoQuality<0) nDemoQuality=0;

   // Determine what type of texture filtering to use at each quality settings
   // At setting 5 or above, use full LINEAR/LINEAR mipmapping
   // At settings below, use simpler types of mipmapping
   if (nDemoQuality>=5)
   {
      glWantMipmap = GL_LINEAR_MIPMAP_LINEAR;
      glWantLinear = GL_LINEAR;
   } else
   if (nDemoQuality>=4)
   {
      glWantMipmap = GL_LINEAR_MIPMAP_NEAREST;
      glWantLinear = GL_LINEAR;
   } else
   if (nDemoQuality>=3)
   {
      glWantMipmap = GL_NEAREST_MIPMAP_NEAREST;
      glWantLinear = GL_LINEAR;
   }
   if (nDemoQuality>=2)
   {
      glWantMipmap = GL_LINEAR;
      glWantLinear = GL_LINEAR;
   }
   else
   {
      glWantMipmap = GL_NEAREST;
      glWantLinear = GL_NEAREST;
   }
}


CEnvInfo::~CEnvInfo()
{
}
