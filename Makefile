CXXFLAGS=$(shell sdl2-config --cflags)
LDFLAGS=$(shell sdl2-config --libs)
raytr: raytr.cpp
	$(CXX) $(CXXFLAGS) -o raytr raytr.cpp $(LDFLAGS)
