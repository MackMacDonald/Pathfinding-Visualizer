OBJ = pv_window.o pv_gui.o pv_algorithms.o
LIB = -L. -lraylib -lopengl32 -lgdi32 -lwinmm
INCLUDE_PATHS = -I.
CFLAGS= -Wall -g -O3
EXECUTABLE_NAME = PathfindingVisualizer

$(EXECUTABLE_NAME): $(OBJ)
	g++ -o $@ $(OBJ) $(INCLUDE_PATHS) $(LIB)

pv_window.o: pv_window.cpp
	g++ -c $< $(CFLAGS) $(INCLUDE_PATHS)

pv_gui.o: pv_gui.cpp
	g++ -c $< $(CFLAGS) $(INCLUDE_PATHS)

pv_algorithms.o: pv_algorithms.cpp
	g++ -c $< $(CFLAGS) $(INCLUDE_PATHS)

clean:
	rm -f $(OBJ) $(EXECUTABLE_NAME)