.POSIX:

# Include path to specific library path first, then common second
# Library path does the same

INCLUDES = -I../vidgl_glut -I../geogl -I../MediaDuke -I../fmod/api -I../common/include
XLIBS    = -L/usr/X11/lib -L/usr/X11R6/lib -lX11 -lXext -lXmu -lXt -lXi -lSM -lICE
GLLIBS   = -L /usr/lib -lglut -lGLU -lGL -lm $(XLIBS)
MOBYLIBS = -L../vidgl_glut -L../geogl -L../MediaDuke -L../glpng/lib -L../common/lib -L. -lgeogl -lvidgl -lmduke -lfmod-3.31 -lglpng
LIBS     = $(MOBYLIBS) $(GLLIBS) -L/usr/local/lib
CFLAGS   = -O -I. $(INCLUDES)
CC       = g++

PROGRAM  = mwnode
OBJECTS  = CComaLogo.o CCubeZoom.o CCubesACM.o CDemoEffect.o CEnvInfo.o CFractalIntro.o \
		CIFS.o CMotherboard.o CParticleEngine.o CShatter.o CSparkles.o CVidGLDerive.o \
		CParticle.o main.o tiler.o

#
# --- GENERIC DEPENDENCIES --- #
#
#
$(PROGRAM):	$(OBJECTS)
		$(CC) -o $@ $(CFLAGS) $(OBJECTS) $(LIBS) -Xlinker -rpath -Xlinker .

%.o:		%.cpp %.h
		$(CC) $(CFLAGS) -c $<

clean:		
		rm -f $(OBJECTS) $(PROGRAM)

main.o:		main.cpp
		$(CC) $(CFLAGS) -c $<

# Include files: need to add dependencies for some of these...
#CComaLogo.h*
#CCubeZoom.h*
#CCubesACM.h*
#CDemoEffect.h*
#CEnvInfo.h*
#CFractalIntro.h*
#CIFS.h*
#CMotherboard.h*
#CParticle.h*
#CParticleEngine.h*
#CShatter.h*
#CSparkles.h*
#CVidGLDerive.h*
#resource.h*
#tiler.h*
