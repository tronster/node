#include "tiler.h"

using namespace geoGL;
using namespace md;

//////////////////////////////////////////////////////////////////////////////
//	CONSTRUCTOR
//
//	-	Set things to NULL zippo values
//////////////////////////////////////////////////////////////////////////////
CTilerGL::CTilerGL()   { nSize = 0; nTilesX = nTilesY = nTiles = 0; image = NULL; }
CTilerGL::~CTilerGL()  { clear(); }

void CTilerGL::clear()
{
   /* do not delete image - it is not ours */
   textures.clear();
   points.clear();
   image = NULL;
}

void CTilerGL::setImage(md::Cimage *useImage)
{
   if (image)
      throw "CTilerGL already has an image";
   image = useImage;
   image->makeDataRGB();
}

//////////////////////////////////////////////////////////////////////////////
// Generate texture tiles
//   - Create textures array
//   - Create points array
//   - Compute nTilesX, nTilesY
//   - Image should be created, size set
//
// PRE:  Image set, tile size set
// POST: textures created, points created
//////////////////////////////////////////////////////////////////////////////
void CTilerGL::createTiles()
{
   // Are boundary tiles needed?
   int nBoundX = (image->x % nSize)==0 ? 0 : 1;
   int nBoundY = (image->y % nSize)==0 ? 0 : 1;

   if (nBoundX+nBoundY > 0)
      throw "CTilerGL: Non-even texture sizes not currently supported";

   // Compute number of tiles across and down
   nTilesX = image->x / nSize + nBoundX;
   nTilesY = image->y / nSize + nBoundY;

   // Create arrays for textures and points
   nTiles = nTilesX * nTilesY;
   textures.refnew(nTiles);
   points.refnew  (nTiles * 4);

   // Create textures and points
   int nTile = 0;
   for (int x=0; x<nTilesX; x++)
      for (int y=0; y<nTilesY; y++) {
         createTile(nTile,x*nSize,y*nSize,nSize,nSize);
         nTile++;
      }

}

//
// Generate a single textured tile
//
void CTilerGL::createTile(int nTile, int x1, int y1, int nWidth, int nHeight)
{
   // Create empty image
   Cimage imageTemp;
   imageTemp.create(nSize,nSize,3);
   memset(imageTemp.data,0,imageTemp.dataSize);

   // Fill it with a texture
   for (int y=0; y<nSize; y++) {
      for (int x=0; x<nSize; x++) {
         int dst = (y*imageTemp.x + x) * imageTemp.bytesPerPixel;
         int src = ((y+y1)*image->x + x + x1) * image->bytesPerPixel;
         imageTemp.data[dst+0] = image->data[src+0];
         imageTemp.data[dst+1] = image->data[src+1];
         imageTemp.data[dst+2] = image->data[src+2];
      }
   }

   points[nTile*4 + 0] = geoGL::ul;
   points[nTile*4 + 1] = geoGL::ur;
   points[nTile*4 + 2] = geoGL::lr;
   points[nTile*4 + 3] = geoGL::ll;

   // Create gl texture from it
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
   textures[nTile] = imageTemp.makeGLTexture(GL_LINEAR,GL_LINEAR);
   if (textures[nTile] == -1)
      throw "CTiler::Cimage reported error creating tile texture";
   if (glGetError() != GL_NO_ERROR)
      throw "CTiler::Opengl error after creating tile texture";
}