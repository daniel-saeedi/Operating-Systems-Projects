COMPILER = g++
COMPILER_FLAGS = --std=c++11
all : school.out classHandler.out student.out course.out

school.out : school.o
	${COMPILER} ${COMPILER_FLAGS} school.o -o school.out
	
school.o :  school.cpp school.h
	${COMPILER} ${COMPILER_FLAGS} -c school.cpp -o school.o

classHandler.out : classHandler.o
	${COMPILER} ${COMPILER_FLAGS} classHandler.o -o classHandler.out

classHandler.o :  classHandler.cpp classHandler.h
	${COMPILER} ${COMPILER_FLAGS} -c classHandler.cpp -o classHandler.o

student.out : student.o
	${COMPILER} ${COMPILER_FLAGS} student.o -o student.out

student.o :  student.cpp student.h
	${COMPILER} ${COMPILER_FLAGS} -c student.cpp -o student.o

course.out : course.o
	${COMPILER} ${COMPILER_FLAGS} course.o -o course.out

course.o :  course.cpp course.h
	${COMPILER} ${COMPILER_FLAGS} -c course.cpp -o course.o

clean:
	rm -rf *.out 