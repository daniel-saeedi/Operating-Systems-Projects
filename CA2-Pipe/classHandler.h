#ifndef CLASS_HANDLER_H
#define CLASS_HANDLER_H

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <vector>

#include <filesystem>

namespace fs = std::__fs::filesystem;

using namespace std;

class ClassProcess{
public:
    ClassProcess(string _class_name,string _dir){
        class_name = _class_name;
        class_name.erase(remove(class_name.begin(), class_name.end(), ' '), class_name.end());
        dir = _dir + "/"+class_name+"/";
    }

    bool create_student_process_recur(vector<string> students,int i = 0){
        
        if(i >= students.size())
            return true;

        string student_name_ = students[i];
        char student_name[1000];
        strcpy(student_name,student_name_.c_str());

        i += 1;
        return ((create_student_process(student_name)) && (create_student_process_recur(students,i)) );
    }

    void run(){
        vector<string> students = get_file_list(dir);
        create_student_process_recur(students);
    }
private:
    string class_name;
    string dir;

    pid_t create_student_process(char *student_name)
    {
        int ret_val;
        int pfd[2];
        char buff[1000];
        char string1[1000];
        strcpy(string1,student_name);

        ret_val = pipe(pfd);                 /* Create pipe */

        pid_t c1_pid;

        c1_pid = fork();

        if (c1_pid == 0) {

            char* arr[] = {NULL};
            close(pfd[1]); /* close the write end of pipe */
            ret_val = read(pfd[0],buff,strlen(string1)); /* Read from pipe */
            if (ret_val != strlen(string1)) {
            printf("Read did not return expected value\n");
            exit(3);                       /* Print error message and exit */
            }

            execl("student.out", "student.out", buff, NULL);
        }
        else if (c1_pid < 0){
            cout << "ERROR!" << endl;
            return 1;          
        }else {
            close(pfd[0]); /* close the read end */
            ret_val = write(pfd[1],string1,strlen(string1)); /*Write to pipe*/
            if (ret_val != strlen(string1)) {
                printf("Write did not return expected value\n");
                exit(2);                       /* Print error message and exit */
            }
            wait(NULL);
        }        

        return c1_pid;
    }

    vector<string> get_file_list(string path){
        vector<string> f;
        for (const auto & file : fs::directory_iterator(path)){
            f.push_back(file.path());
            // cout << file.path() << endl;
        }
        return f;
    }
};

#endif