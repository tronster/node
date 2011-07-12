#include "geometry3d.h"
#include <set>

//
// Class to hold information for an IFS recursive step
//
class IFSStep {
public:
   geoGL::fVector2D  point;
   int               recurse;
   int               rot;
};

//
// Class to store a line segment
//
class IFSLine {
public:
   geoGL::fVector2D  start,end;
   int               nDepth;

   bool closeTo(const IFSLine &rhs) const;
};

//
// STL binary_function for ordering Lines by depth
//
template<class IFSLine>
struct DepthLess : public std::binary_function<IFSLine, IFSLine, bool>
{
   inline bool operator()(const IFSLine& lhs, const IFSLine& rhs) const { 
      return (lhs.nDepth < rhs.nDepth);
   }
};

//
// STL binary_function for ordering Lines by coordinate position
//
template<class IFSLine>
struct CoordLess : public std::binary_function<IFSLine, IFSLine, bool>
{
   bool operator ()(const IFSLine &lhs, const IFSLine &rhs) const;
};

// Shortcuts for various line sets used in IFS class
typedef std::multiset<IFSLine, CoordLess<IFSLine> >     line_Set;
typedef std::multiset<IFSLine, DepthLess<IFSLine> >     lineDepth_Set;
typedef line_Set::iterator                        line_SetIT;
typedef lineDepth_Set::iterator                   lineDepth_SetIT;

//
// IFS fractals - converted from Turbo Pascal 6.0/DOS
//
class CIFS {
private:
   #define nTABLESIZE         360                           // Size of sine/cosine tables
   #define fSKIPVALUE         (2.0f*PI/nTABLESIZE)
   #define nCOLORSIZE         16

   static int                 sinTable[nTABLESIZE];         // sine/cosine tables for speed
   static int                 cosTable[nTABLESIZE];
   static bool                bTableCreated;
   static int                 nPruningLevel;

   // Working arrays during point generation
   // These are cleared after CopyToArrays()
   line_Set                   lineSet;
   geoGL::fVector2D           lineStart,lineEnd;            // Currently drawn line

   // Final arrays after pruning and output
   // These are filled after CopyToArrays()
   int                        nPoints, nColors;             // Number of points, colors
   refptr<geoGL::fVector2D>   pPoints;
   refptr<int>                pDepths;                      // Indices of points at particular depths

   // Parameters describing the fractal tree
   int                        numPoints,                    // Number of IFS steps
                              maxDepth,                     // Iteration depth
                              scale,                        // Scaling information
                              depthScale,                   // Computed scale relative to depth
                              shade,                        // Color shading divisor (why?)
                              currDepth, currPts;           // Current depth, Current save point
   geoGL::fRGBA               pColors[nCOLORSIZE];          // Simple color table for fractal
   refptr<IFSStep>            points;                       // steps for generating fractal

   // Create sin/cos tables
   static void createTable();

   // Rotate a point in 2D space
   void rot2D(int rot, geoGL::fVector2D in, geoGL::fVector2D &p);

   // Main recursive rendering phase
   void render(geoGL::fVector2D p, int rot);

   // Virtual turtle drawing functions that draw to arrays
   void moveTo(geoGL::fVector2D d);
   void lineTo(geoGL::fVector2D d);

public:
   CIFS();
	~CIFS();

   // Initialize IFS fractal parameters
   void init(int nDepth, int nScale, int nShade, int nPoints, geoGL::fVector2D startPos);

   // Clear everything, including the finalized output
   void clear();

   // Place color gradient into table
   void colorGradient(float r1, float g1, float b1, float r2, float g2, float b2,
                      int start, int stop);

   // Setup point n for fractal design
   void setPoint  (int n, geoGL::fVector2D p, int nRecursionLevel, int nRotation);

   // Get/Set pruning level (5 is default, higher is less detail)
   static void       setPruning(int nNewPruning)   { nPruningLevel = nNewPruning; }
   static inline int getPruning()                  { return nPruningLevel; }

   // Begin rendering fractal at specific angle
   void render(int rot=0);

   // Post-process rendering output
   void prune();
   void copyToArrays();

   // Accessors to finalized output
   inline int                              getNumPoints()    const   { return nPoints; }
   inline const refptr<geoGL::fVector2D> & getPoints()       const   { return pPoints; }
   inline const refptr<int>              & getDepthIndices() const   { return pDepths; }
   inline int                              getMaxDepth()     const   { return maxDepth; }
   inline const geoGL::fRGBA             * getColors()       const   { return pColors; }
};