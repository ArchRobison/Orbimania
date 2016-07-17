VPATH = ../../../Source ../../../Platform/SDL-2.0

CPLUS = clang++ -std=c++11 -MMD
#CPLUS_FLAGS = -O0 -g
CPLUS_FLAGS = -O2 -DEXCLUSIVE_MODE -DASSERTIONS=0

INCLUDE = -I../../../Source -I/Library/Frameworks/SDL2.framework/Headers/ -I/Library/Frameworks/SDL2_image.framework/Headers -I/Library/Frameworks/SDL2_ttf.framework/Headers

LIB = \
-framework CoreAudio \
-framework AudioUnit \
-framework CoreFoundation \
-framework CoreGraphics \
-framework CoreServices \
-framework ForceFeedback \
-framework IOKit \
-framework AppKit \
-framework Carbon \
-framework ImageIO \
-framework ApplicationServices \
-liconv 

LIB += -framework SDL2 -framework SDL2_image -framework SDL2_ttf

%.o: %.cpp
	$(CPLUS) $(CPLUS_FLAGS) -c $(INCLUDE) $<

OBJ = Arrow.o AssertLib.o BuiltFromResource.o Circle.o Clut.o ColorMatrix.o \
	FuturePath.o Game.o Handle.o Menu.o NimbleDraw.o \
    PotentialFieldBarnesHut.o PotentialFieldBilinear.o PotentialFieldPrecise.o \
	Render.o TimeStep.o Universe.o View.o Host_sdl.o

orbimania: $(OBJ)
	$(CPLUS) $(CPLUS_FLAGS) -o $@ $(OBJ) $(LIB)

clean:
	rm -f *.[od] orbimania

*.o: Makefile

-include *.d
