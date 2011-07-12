#pragma warning(disable:4786)
#include "CIFS.h"

using namespace geoGL;
using namespace std;

/*********************************** Line class ********************************/
bool IFSLine::closeTo(const IFSLine &rhs) const {
   if ((rhs.start-this->start).length() >= CIFS::getPruning() || (rhs.end-this->end).length() >= CIFS::getPruning())
      return false;
   else
      return true;
}

template <class IFSLine>
bool CoordLess<IFSLine>::operator ()(const IFSLine &lhs, const IFSLine &rhs) const
{
   if (lhs.start.x < rhs.start.x) return true;
   if (lhs.start.y < rhs.start.y) return true;
   if (lhs.end.x   < rhs.end.x) return true;
   if (lhs.end.y   < rhs.end.y) return true;
   return false;
}


// Static members
int      CIFS::sinTable[nTABLESIZE];
int      CIFS::cosTable[nTABLESIZE];
bool     CIFS::bTableCreated = false;
int      CIFS::nPruningLevel = 5;

// Constructors
CIFS::CIFS() {}
CIFS::~CIFS() {}

void CIFS::init(int _depth, int _scale, int _shade, int _num, fVector2D _start) {
   maxDepth   = _depth;
   scale      = _scale;
   depthScale = maxDepth/scale;
   shade      = _shade;
   numPoints  = _num;
   lineStart = _start;

   points.refnew(numPoints);

   createTable();
}

void CIFS::clear()
{
   lineSet.clear();
   points.clear();
   pPoints.clear();
   pDepths.clear();
}

void CIFS::colorGradient(float r1, float g1, float b1, float r2, float g2, float b2,
                        int start, int stop)
{
   float range = (stop-start+1);
   float dr = (r2-r1)/range;
   float dg = (g2-g1)/range;
   float db = (b2-b1)/range;

   for (int c=0; c<range; c++)
      pColors[c] = fRGBA((r1+dr*c),(g1+dg*c),(b1+db*c));
}

// Create sin/cos tables
void CIFS::createTable()
{
   if (bTableCreated)
      return;

   for (int off=0; off<nTABLESIZE; off++) {
      sinTable[off] = (int) (sin(off*fSKIPVALUE)*32768);
      cosTable[off] = (int) (cos(off*fSKIPVALUE)*32768);
   }

   bTableCreated = true;
}

void CIFS::rot2D(int rot, fVector2D in, fVector2D &p) {
   int newx1, newy1;
   int sinZrot, cosZrot;

   sinZrot = sinTable[rot];  cosZrot =cosTable[rot];
   newx1   = (int) (in.x*cosZrot - in.y*sinZrot) >> 15;
   newy1   = (int) (in.x*sinZrot + in.y*cosZrot) >> 15;
   p.x = newx1*(maxDepth-currDepth)/depthScale;
   p.y = newy1*(maxDepth-currDepth)/depthScale;
}

void CIFS::setPoint  (int n, fVector2D p, int nRecursionLevel, int nRotation) {
   points[n].point   = p;
   points[n].recurse = nRecursionLevel;
   points[n].rot     = nRotation;
}

void CIFS::render(int rot) {
   currDepth=0;
   currPts=0;
   lineSet.clear();
   render(lineStart, rot);
}

void CIFS::render(fVector2D p, int rot) {
   fVector2D temp(0,0);

   for (int i=0; i<numPoints; i++) {
      moveTo(temp + p);

      rot2D(rot, points[i].point, temp);

      lineTo(temp+p);
      currPts++;

      currDepth++;
      if (currDepth<maxDepth && points[i].recurse>0)
         if (currDepth % points[i].recurse==0)
            render(temp+p, (rot+points[i].rot) % nTABLESIZE);
      currDepth--;
   }
}

void CIFS::prune() {}

//
// Copies internal storage vectors to arrays
// Deallocates vectors, etc.
//
void CIFS::copyToArrays()
{
   lineDepth_Set     lineDepthSet;        // Set for storing points orderd by depth
   lineDepth_SetIT   lineDepthSetIT;      // Iterator for depth ordered set
   line_SetIT        lineSetIT;           // Iterator for point ordered set

   // Copy data from lineSet to lineDepthSet
   // This reorders it by depth
   for (lineSetIT=lineSet.begin(); lineSetIT!=lineSet.end(); lineSetIT++)
      lineDepthSet.insert(*lineSetIT);

   // Allocate array for points
   nPoints = lineDepthSet.size()*2;
   pPoints.refnew(nPoints);

   // Allocate array for depth indices - extra index = nPoints
   pDepths.refnew(maxDepth+1);
   pDepths[maxDepth] = nPoints;

   // Copy data from lineDepthSet to arrays for final output
   int nIndex, nDepth;
   nIndex = 0;
   nDepth = 0;

   for (lineDepthSetIT=lineDepthSet.begin(); lineDepthSetIT!=lineDepthSet.end(); lineDepthSetIT++)
   {
      if ((*lineDepthSetIT).nDepth == nDepth) {       // Fill depth index table
         pDepths[nDepth] = nIndex;
         nDepth++;
      }

      pPoints[nIndex+0] = (*lineDepthSetIT).start;    // Copy points
      pPoints[nIndex+1] = (*lineDepthSetIT).end;

      nIndex += 2;
   }

   // No longer need original set of points
   lineSet.clear();
   points.clear();
}

// Turtle drawing functions: moveTo/lineTo -- they add to the lineSet

//
// Set starting point
//
void CIFS::moveTo(fVector2D d)
{
   lineStart = d;
}

//
// Set ending point, write to set
//   -- Do not write duplicates or "similar" lines
//
void CIFS::lineTo(fVector2D d)  
{ 
   // Line ends at this point
   lineEnd = d; 

   // If the endpoints match, there is no line
   if (lineEnd==lineStart)
      return;

   // Create the line structure
   IFSLine newLine;
   newLine.start = fVector2D(lineStart.x,lineStart.y);
   newLine.end   = fVector2D(lineEnd.x,  lineEnd.y);
   newLine.nDepth= currDepth;

   pair<line_SetIT,line_SetIT> nearest = lineSet.equal_range(newLine);
   try {
      if (newLine.closeTo(*nearest.first) || newLine.closeTo(*nearest.second)) 
         return;
   } catch (...) {
      // Tried to derefence null iterator
      // This means there is a missing neighbor
   }

   lineSet.insert(nearest.second,newLine);
   lineStart = lineEnd;
}
