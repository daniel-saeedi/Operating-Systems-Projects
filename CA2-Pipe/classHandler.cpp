#include "classHandler.h"

int main(int argc, char *argv[]){
    // string filename("a.txt");
    // ofstream file_out;

    // file_out.open(filename, std::ios_base::app);
    // file_out << "Some random text to append." << endl;
    // cout << "Done !" << endl;


    // int ret_val;
    // int pfd[2];
    // char buff[32];
    // char string1[30];
    // ret_val = pipe(pfd);                 /* Create pipe */

    // close(pfd[1]); /* close the write end of pipe */
    // ret_val = read(pfd[0],buff,strlen(string1)); /* Read from pipe */
    // if (ret_val != strlen(string1)) {
    //     printf("Read did not return expected value\n");
    //     exit(3);                       /* Print error message and exit */
    // }

    ClassProcess cp(argv[1],argv[2]);
    cp.run();
    return 0;
}