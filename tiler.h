#ifndef __CTilerGL_h__
#define __CTilerGL_h__

#include "geometry3d.h"
#include "mduke.h"

class CTilerGL
{
private:
   md::Cimage                   *image;            // Image to tile
   int                           nTiles;           // # of tiles
   int                           nTilesX,nTilesY;  // # of tiles in each dimension
   int                           nSize;            // nSize x nSize textures dimensions

public:
   refptr<geoGL::TextureID>      textures;         // Array of textures to use
   refptr<geoGL::fVector2D>      points;           // Array of texture points

   CTilerGL();
   ~CTilerGL();

   void  createTiles();
   void  clear();

          void  setImage(md::Cimage *useImage);
   inline void  setTileSize(int nSetSize)       { nSize = nSetSize; }
   inline int   getTileSize()                   { return nSize;     }
   inline int   getTilesX()                     { return nTilesX;   }
   inline int   getTilesY()                     { return nTilesY;   }
   inline int   getNumTiles()                   { return nTiles;    }
private:
   void createTile(int nTexNumber, int x1, int y1, int nWidth, int nHeight);
};

#endif __CTilerGL_h__