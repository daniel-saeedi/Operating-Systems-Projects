#ifndef COURSE_H
#define COURSE_H

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

class CourseProcess{
public:
    CourseProcess(string _course_name,string _number_of_students){
        course_name = split_string(_course_name,",")[0];
        number_of_students = atoi(_number_of_students.c_str());
    }

    void run()
    {
        read_grade();
    }

    void read_grade(){
        int fd1;
        // FIFO file path
        string FIFO_file_path = "tmp/"+course_name;
        char * myfifo;
        strcpy(myfifo,FIFO_file_path.c_str());

        // Creating the named file(FIFO)
        // mkfifo(<pathname>,<permission>)
        mkfifo(myfifo, 0666);

        int i = 0;
        while(i < 11){
            i++;
            char str1[100], str2[100];
            fd1 = open(myfifo,O_RDONLY);
            read(fd1, str1, 100);

            total_grade += atoi(str1);
            
            close(fd1);
        }
        cout << " Course: " << course_name << " Grade: " << total_grade/number_of_students << endl;
    }

private:
    string course_name;

    int total_grade;
    int number_of_students;

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