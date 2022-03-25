#include Make_linux.inc
#include Make_intel.inc
include Make_msys2.inc
#include Make_osx.inc

CXXFLAGS = -std=c++11 -fPIC
ifdef DEBUG
CXXFLAGS += -g -O0 -Wall -fbounds-check -pedantic -D_GLIBCXX_DEBUG
else
CXXFLAGS += -O3 -march=core-avx2 -Wall
endif

default:	help

all:	filtres_images.exe

filtres_images.exe:	filtres_images.cpp lodepng/lodepng.cpp
	$(CXX) $(CXXFLAGS) -o filtres_images.exe filtres_images.cpp lodepng/lodepng.cpp $(LIB)

help: 
	@echo "Available targets : "
	@echo "    all                               : compile all executables"
	@echo "    filtres_images.exe                : compile filtres_images.exe executable"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(CXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"

clean:
	@rm -f *.exe *~ 

