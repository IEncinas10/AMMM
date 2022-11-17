CXX    =  g++
FLAGS  = -g -O3 -DNDEBUG
LFLAGS = -lfmt

ALL	= solver tuner
all: $(ALL)

tuner: alphaTuner.cpp algorithms.h
	$(CXX) alphaTuner.cpp $(FLAGS) $(LFLAGS) -o $@

solver: solver.cpp algorithms.h
	$(CXX) solver.cpp $(OPT3) $(FLAGS) $(LFLAGS) -o $@

clean:
	rm $(ALL)


