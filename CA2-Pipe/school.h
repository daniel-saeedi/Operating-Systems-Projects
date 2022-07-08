#ifndef SCHOOL_H
#define SCHOOL_H

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h> 
#include <dirent.h>
#include <fcntl.h>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <cstring>


// #include "classHandler.h"

using namespace std;

namespace fs = std::__fs::filesystem;


class SchoolProcess{
public:
    SchoolProcess(string _dir)
    {
        dir = _dir;
    }

    bool create_class_process_recur(int class_count,int i = 0){
    
        if(i >= class_count)
            return true;
        string folder = "class" + to_string(i+1) + " ";
        char class_id[100];
        strcpy(class_id,folder.c_str());
        i += 1;
        return ((create_class_process(class_id)) && (create_class_process_recur(class_count,i)) );
    }

    bool create_course_process_recur(vector<string> courses,int i = 0){
        
        if(i >= courses.size())
            return true;
    
        string _course_name = courses[i];
        char course_name[100];

        strcpy(course_name,_course_name.c_str());
        i += 1;

        return ((create_course_process(course_name)) && 
                (create_course_process_recur(courses,i)) );
    }

    void run()
    {
        vector<string> courses;
        courses.push_back("Physics,");
        courses.push_back("English,");
        courses.push_back("Math,");
        courses.push_back("Literature,");
        courses.push_back("Chemistry,");

        pid_t c1_pid,c2_pid;
        (c1_pid = fork()) && (c2_pid = fork());

        if (c1_pid == 0) {
            create_course_process_recur(courses);            
        }else if(c2_pid == 0){
            vector<string> dirs = get_directories(dir);
            int number_of_classes = dirs.size();
            create_class_process_recur(3);
        }
        else {
            wait(NULL);
        }

        
    }
private:
    string dir;

    pid_t create_class_process(char *c_name)
    {
        int ret_val;
        int pfd[2];
        char buff[32];
        char string1[30];
        memset(&string1, 0, sizeof(string1));
        strcpy(string1,c_name);

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

            execl("classHandler.out", "classHandler.out", buff,dir.c_str(), NULL);
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

    pid_t create_course_process(char string1[1000])
    {
        int ret_val;
        int pfd[2];
        char buff[1000];

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

            execl("course.out", "course.out", buff, NULL);
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



    vector<string> get_directories(string path)
    {
        vector<string> folders;
        for (const auto & entry : fs::directory_iterator(path))
            folders.push_back(entry.path());
        return folders;
    }
};

#endif