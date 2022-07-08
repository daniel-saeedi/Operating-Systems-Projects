#ifndef STUDENT_H
#define STUDENT_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <stdio.h>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

class StudentProcess{
public:
    StudentProcess(string _grade_path){
        grade_path = _grade_path;
    }

    void run()
    {
        vector<string> courses;
        vector<string> grades;
        std::ifstream file(grade_path);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                vector<string> param = split_string(line,",");
                string course_name = param[0];
                string grade = param[1];
                // 

                courses.push_back(course_name);
                grades.push_back(grade);
            }
            file.close();
        }

        pid_t c1_pid, c2_pid, c3_pid, c4_pid, c5_pid;

        (c1_pid = fork()) && (c2_pid = fork()) && (c3_pid = fork()) && (c4_pid = fork()) && (c5_pid = fork()); // Creates two children

        if (c1_pid == 0) {
            send_grade_to_course_process(courses[0],grades[0]);
        } else if (c2_pid == 0) {
            send_grade_to_course_process(courses[1],grades[1]);
        } else if (c3_pid == 0) {
            send_grade_to_course_process(courses[2],grades[2]);
        } else if (c4_pid == 0) {
            send_grade_to_course_process(courses[3],grades[3]);
        } else if (c5_pid == 0) {
            send_grade_to_course_process(courses[4],grades[4]);
        } else {
            wait(NULL);
        }
    }

    void send_grade_to_course_process(string course_name, string grade)
    {
        int fd;

        // FIFO file path
        string FIFO_file_path = "tmp/"+course_name;

        char * myfifo;
        strcpy(myfifo, FIFO_file_path.c_str());

        // Creating the named file(FIFO)
        // mkfifo(<pathname>, <permission>)
        mkfifo(myfifo, 0666);

        char arr2[100];
        strcpy(arr2,grade.c_str());
        while(1){
            // Open FIFO for write only
            fd = open(myfifo, O_WRONLY);

            // Write the input arr2ing on FIFO
            // and close it
            if(write(fd, arr2, strlen(arr2)+1) > 0){
                close(fd);
                break;
            };
            close(fd);
        }
        
    }
private:
    string grade_path;

    std::vector<std::string> split_string(const std::string& str,
                                      const std::string& delimiter)
    {
        std::vector<std::string> strings;

        std::string::size_type pos = 0;
        std::string::size_type prev = 0;
        while ((pos = str.find(delimiter, prev)) != std::string::npos)
        {
            strings.push_back(str.substr(prev, pos - prev));
            prev = pos + 1;
        }

        // To get the last substring (or only, if delimiter is not found)
        strings.push_back(str.substr(prev));

        return strings;
    }
};

#endif