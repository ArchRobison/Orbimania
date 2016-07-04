VPATH = ../../../Source

CPLUS = clang++ -std=c++11 
OPT = -O2

%.o: %.cpp
	$(CPLUS) $(OPT) -c $<

OBJ = Arrow.o AssertLib.o BuiltFromResource.o Circle.o Clut.o ColorMatrix.o \
	Game.o Handle.o NimbleDraw.o PotentialFieldBarnesHut.o PotentialFieldBilinear.o \
	Render.o TimeStep.o Universe.o View.o

orbimania: $(OBJ)
	$(CPLUS) $(OPT) $(OBJ) -o $@
