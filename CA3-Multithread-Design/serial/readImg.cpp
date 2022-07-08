#include <iostream>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include <math.h>
#include <algorithm>
#include <iomanip>
#include <vector>

#include "typedefs.hpp"

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using namespace std;
using namespace std::chrono;

int rows;
int cols;
RGBS pixels;
RGBS original_pixels;

float total_time;

void mirror_horizontally();
void mirror_vertical();
void invert();
void median();
void plus_mark();


bool fillAndAllocate(char *&buffer, const char *fileName, int &rows, int &cols, int &bufferSize)
{
  std::ifstream file(fileName);

  if (file)
  {
    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    return 1;
  }
  else
  {
    cout << "File " << fileName << " doesn't exist!" << endl;
    return 0;
  }
}

void getPixlesFromBMP24(int end, int rows, int cols, char *fileReadBuffer)
{
  int count = 1;
  int extra = cols % 4;

  for (int i = 0; i < rows; i++)
  {
    count += extra;
    for (int j = cols - 1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        switch (k)
        {
        case 0:
          original_pixels[i][j][RED] = fileReadBuffer[end - count];
          break;
        case 1:
          original_pixels[i][j][GREEN] = fileReadBuffer[end - count];
          break;
        case 2:
          original_pixels[i][j][BLUE] = fileReadBuffer[end - count];
          break;
        }
        count++;
      }
  }
}

void writeOutBmp24(char *fileBuffer, const string &nameOfFileToCreate, int bufferSize)
{
  std::ofstream write(nameOfFileToCreate);
  if (!write)
  {
    cout << "Failed to write " << nameOfFileToCreate << endl;
    return;
  }
  int count = 1;
  int extra = cols % 4;
  for (int i = 0; i < rows; i++)
  {
    count += extra;
    for (int j = cols - 1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        switch (k)
        {
        case 0:
          fileBuffer[bufferSize - count] = pixels[i][j][RED];
          break;
        case 1:
          fileBuffer[bufferSize - count] = pixels[i][j][GREEN];
          break;
        case 2:
          fileBuffer[bufferSize - count] = pixels[i][j][BLUE];
          break;
        }
        count++;
      }
  }
  write.write(fileBuffer, bufferSize);
}

void initialize_pixels()
{
  pixels = new unsigned char**[rows];
  original_pixels = new unsigned char**[rows];
  for (int i = 0; i < rows; i++) {
    pixels[i] = new unsigned char*[cols];
    original_pixels[i] = new unsigned char*[cols];
    for (int j = 0; j < cols; j++) {
      pixels[i][j] = new unsigned char[channels];
      original_pixels[i][j] = new unsigned char[channels];
    }
  }
}

void set_color_value(int row, int col, const int color[])
{
  for (int i = 0; i < channels; i++)
    pixels[row][col][i] = color[i];
}

void update_original_pixels()
{
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      for (int k = 0; k < channels; k++)
        original_pixels[i][j][k] = pixels[i][j][k];
    }
  }
}

void run_filters()
{
  for (int i = 0; i < TOTAL_FILTERS; i++) {
    auto start = high_resolution_clock::now();
    switch (i)
    {
    case MIRROR_X:
      mirror_horizontally();
      break;
    case MIRROR_Y:
      mirror_vertical();
      break;
    case MEDIAN:
      median();
      break;
    case INVERT:
      invert();
      break;
    case PLUS_MARK:
      plus_mark();
      break;
    }

    // This part is taken from stackoverflow
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Execution Time: " << std::fixed << std::setprecision(2)<< duration.count()/1000.0 << " ms" << endl;
    
    total_time += duration.count()/1000.0;

    start = high_resolution_clock::now();

    update_original_pixels();  
  }

  return;
}

void filter_serial(char *fileBuffer, int bufferSize, char *fileName)
{
  auto serial_start = high_resolution_clock::now();

  auto start = high_resolution_clock::now();

  getPixlesFromBMP24(bufferSize, rows, cols, fileBuffer);

  run_filters();

  writeOutBmp24(fileBuffer, "output.bmp", bufferSize);

  cout << "Total Execution Time: " 
                                              << std::fixed
                                              << std::setprecision(2)
                                              << total_time << " ms" << endl;

  return;
}

void mirror_vertical()
{
  cout << "Mirror X initiated" << endl;

	for (int i = 0; i < rows/2; i++) {
		for (int j = 0; j < cols; j++){
      for (int k = 0; k < channels; k++) {

          int temp1 = original_pixels[i][j][k];
          int temp2 =  original_pixels[rows-i-1][j][k];

          //Swap temp1 and temp2

          pixels[i][j][k] = temp2;
          pixels[rows-i-1][j][k] = temp1;

          // cout << j << " " << cols-j-1 << endl;
      }
    }
    // exit(0);
  }


  return;
}

void mirror_horizontally()
{
  cout << "Mirror X initiated" << endl;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols/2; j++){
      for (int k = 0; k < channels; k++) {

          int temp1 = original_pixels[i][j][k];
          int temp2 =  original_pixels[i][cols-j-1][k];

          

          // //Swap temp1 and temp2

          pixels[i][j][k] = temp2;
          pixels[i][cols-j-1][k] = temp1;

          // cout << j << " " << cols-j-1 << endl;
      }
    }
  }
  return;
}

bool is_valid(int i,int j)
{
    return i < rows && j < cols && i >= 0 && j >= 0;
}

double calc_median(vector<int> vec)
{
        typedef vector<int>::size_type vec_sz;

        vec_sz size = vec.size();
        if (size == 0)
                throw domain_error("median of an empty vector");

        sort(vec.begin(), vec.end());

        vec_sz mid = size/2;

        return size % 2 == 0 ? (vec[mid] + vec[mid-1]) / 2 : vec[mid];
}

int find_median(int x, int y, int color)
{
    int col_moves[9] = {-1,0,1,-1,0,1,-1,0,1};
    int row_moves[9] = {-1,-1,-1,0,0,0,1,1,1};

    std::vector<int> numbers;
    for(int i = 0; i < 9; i++){
        if (is_valid(x + row_moves[i],y + col_moves[i]))
          numbers.push_back(original_pixels[x + row_moves[i]][y + col_moves[i]][color]);
    }

    return calc_median(numbers);
}

void median()
{
  cout << "Initiating median filter" << endl;

  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++){
      for (int k = 0; k < channels; k++) {
        pixels[i][j][k] = find_median(i,j,k);
      }
    }


  return;
}

void invert()
{
  cout << "Initiating Invert filter" << endl;

	for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++){
      for (int k = 0; k < channels; k++) {
        pixels[i][j][k] = 255-pixels[i][j][k];
      }
    }


  
    return;
}

void plus_mark()
{
  cout << "Initiating Invert filter" << endl;

  int center_x = cols/2;
  int center_y = rows/2;

  for(int i = 0; i < rows; i++){
    set_color_value(i,center_x,WHITE);
  }

  for(int j = 0; j < cols; j++){
    set_color_value(center_y,j,WHITE);
  }


  
    return;
}


int main(int argc, char *argv[])
{
  char *fileBuffer;
  int bufferSize;
  char *fileName = argv[1];
  if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
  {
    cout << "File read error" << endl;
    return 1;
  }

  initialize_pixels();

  filter_serial(fileBuffer, bufferSize, fileName);
  
  return 0;
}