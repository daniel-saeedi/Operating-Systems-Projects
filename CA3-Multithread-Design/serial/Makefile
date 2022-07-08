CC=g++
STD=-std=c++11
CF=$(STD)

ImageFilters.out: readImg.o
	$(CC) $(CF) $(LINK_FLAGS) -o ImageFilters.out readImg.o

readImg.o: readImg.cpp typedefs.hpp
	$(CC) $(CF) $(LINK_FLAGS) -c readImg.cpp -o readImg.o

.PHONY: clean
clean:
	rm -rf *.o *.out