ROOTCFLAGS  := $(shell root-config --cflags)
ROOTLIBS    := $(shell root-config --libs)

CXX         := g++
CXXFLAGS    := -O2 -fPIC -Wall $(ROOTCFLAGS)
LDFLAGS     := -shared

OBJS = TTrain.o TTrainDict.o
LIB  = libTTrain.so

all: $(LIB)

$(LIB): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(ROOTLIBS)

TTrain.o: TTrain.cxx TTrain.h
	$(CXX) $(CXXFLAGS) -c TTrain.cxx

TTrainDict.cxx: TTrain.h LinkDef.h
	rootcling -f $@ -c TTrain.h LinkDef.h

TTrainDict.o: TTrainDict.cxx
	$(CXX) $(CXXFLAGS) -c TTrainDict.cxx

clean:
	rm -f *.o *.so TTrainDict.*
