#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

int main(int argc, char *argv[])
{
    int arg1, arg2;

    if(argc > 2) {
        arg1 = atoi(argv[1]);
        arg2 = atoi(argv[2]);
    }
    else {
        printf(1, "Invalid inputs for sem_init\n");
        exit();
    }

    sem_init(arg1, arg2);

    exit();
}