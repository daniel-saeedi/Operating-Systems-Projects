#include "student.h"

int main(int argc, char *argv[]){
    StudentProcess sp(argv[1]);

    sp.run();
    return 0;
}