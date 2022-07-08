#include "course.h"

int main(int argc, char *argv[]){
    CourseProcess cp(argv[1],argv[2]);
    cp.run();
}