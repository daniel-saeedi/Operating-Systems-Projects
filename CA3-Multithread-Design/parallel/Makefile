CC=g++
STD=-std=c++11
CF=$(STD)
LINK_FLAGS = -pthread

ImageFilters.out: readImg.o
	$(CC) $(CF) $(LINK_FLAGS) -o ImageFilters.out readImg.o

readImg.o: readImg.cpp
	$(CC) $(CF) $(LINK_FLAGS) -c readImg.cpp -o readImg.o

.PHONY: clean
clean:
	rm -rf *.o *.out