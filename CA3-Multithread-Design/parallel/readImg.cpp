#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <iomanip> 
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>

#include "typedefs.hpp"

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using namespace std;
using namespace std::chrono;

float total_time;

struct Row
{
    int count;
    int end;
    int cols;
    char *fileReadBuffer;
    RGBS pixels;
};

void initialize_pixels_threads(int NUMBER_OF_THREADS, int rows, int cols, RGBS* &pixels_threads)
{
    pixels_threads = new RGBS[NUMBER_OF_THREADS];
    for (int tid = 0; tid < NUMBER_OF_THREADS; tid++)
    {
        pixels_threads[tid] = new unsigned char**[rows];
        for (int i = 0; i < rows; i++) {
            pixels_threads[tid][i] = new unsigned char*[cols];
            for (int j = 0; j < cols; j++) {
                pixels_threads[tid][i][j] = new unsigned char[channels];
            }
        }
    }
}

void* getImg(void* rows)
{
    struct Row curr_row = *(struct Row *) rows;
    for (int i = 0; i < TH_ROW; i++)
    {
        for (int j = curr_row.cols - 1; j >= 0; j--)
            for (int k = 0; k < 3; k++)
            {
                switch (k)
                {
                case RED:
                    curr_row.pixels[i][j][RED] = curr_row.fileReadBuffer[curr_row.end - curr_row.count];
                    break;
                case GREEN:
                    curr_row.pixels[i][j][GREEN] = curr_row.fileReadBuffer[curr_row.end - curr_row.count];
                    break;
                case BLUE:
                    curr_row.pixels[i][j][BLUE] = curr_row.fileReadBuffer[curr_row.end - curr_row.count];
                    break;
                }
                curr_row.count++;
            }
    }
}


int rows;
int cols;
unsigned char avg[channels];

RGBS original_pixels;
RGBS* pixels_threads;
RGBS* pixels_threads_2;

// Filters
void* mirror_horizontally(void *tid);
void* mirror_vertical(void *tid);
void* median(void *tid);
void* invert(void *tid);
void plus_mark();

// This function initializes pixel values in original_pixels
void initialize_pixels()
{
  original_pixels = new unsigned char**[rows];
  for (int i = 0; i < rows; i++) {
    original_pixels[i] = new unsigned char*[cols];
    for (int j = 0; j < cols; j++) {
      original_pixels[i][j] = new unsigned char[channels];
    }
  }
}

void set_color_value(int row, int col, const int color[])
{
  for (int i = 0; i < channels; i++)
    original_pixels[row][col][i] = color[i];
}

void update_original_pixels()
{
  for (int tid = 0; tid < rows/TH_ROW; tid++)
  {
    for (int i = 0; i < TH_ROW; i++)
    {
      for (int j = 0; j < cols; j++)
      {
        for (int k = 0; k < channels; k++)
          original_pixels[tid*TH_ROW + i][j][k] = pixels_threads[tid][i][j][k];
      }
    }
  }
}

void update_original_pixels_col()
{
  for (int tid = 0; tid < cols/TH_COL; tid++)
  {
    for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < TH_COL; j++)
      {
        for (int k = 0; k < channels; k++)
          original_pixels[i][tid*TH_COL + j][k] = pixels_threads_2[tid][i][j][k];
      }
    }
  }
}

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

// Also this part is reimplemented with threads
void getPixlesFromBMP24(int end, int rows, int cols, char *fileReadBuffer)
{
  int count = 1;
  int extra = cols % 4;
  int NUMBER_OF_THREADS = rows / TH_ROW;

  pthread_t threads[NUMBER_OF_THREADS];

  struct Row thread_rows[NUMBER_OF_THREADS];

  for (int tid = 0; tid < NUMBER_OF_THREADS; tid++)
  {
    count += extra;

    thread_rows[tid] = {
      count,
      end,
      cols,
      fileReadBuffer,
      pixels_threads[tid]
    };
    count += TH_ROW*cols*channels;
  }

  for (int tid = 0; tid < NUMBER_OF_THREADS; tid++)
    pthread_create(&threads[tid], NULL, getImg, &thread_rows[tid]);

  for (int tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    pthread_join(threads[tid], NULL);
    
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
          fileBuffer[bufferSize - count] = original_pixels[i][j][RED];
          break;
        case 1:
          fileBuffer[bufferSize - count] = original_pixels[i][j][GREEN];
          break;
        case 2:
          fileBuffer[bufferSize - count] = original_pixels[i][j][BLUE];
          break;
        }
        count++;
      }
  }
  
  write.write(fileBuffer, bufferSize);
}

void convert_to_threads(void *(*filter) (void *))
{
  int NUMBER_OF_THREADS = (rows / TH_ROW);
  pthread_t threads[NUMBER_OF_THREADS];

  for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    pthread_create(&threads[tid], NULL, filter, (void*)tid);

  for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    pthread_join(threads[tid], NULL);
}

void convert_to_threads_col(void *(*filter) (void *))
{
  int NUMBER_OF_THREADS = (cols / TH_COL);
  pthread_t threads[NUMBER_OF_THREADS];

  for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    pthread_create(&threads[tid], NULL, filter, (void*)tid);

  for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    pthread_join(threads[tid], NULL);
}

void run_filters()
{
  for (int i = 0; i < TOTAL_FILTERS; i++) {
    bool update_real = true;
    bool update_real_mirror_y = false;
    auto start = high_resolution_clock::now();
    switch (i)
    {
    case MIRROR_X:
      cout << "Intiating Horizontal Filter filter" << endl;
      convert_to_threads(mirror_horizontally);
      break;
    case MIRROR_Y:
      cout << "Intiating vertical filter" << endl;
      convert_to_threads_col(mirror_vertical);
      update_real_mirror_y = true;
      break;
    case MEAN:
      cout << "Intiating median filter" << endl;
      convert_to_threads(median);
      break;
    
    case INVERT:
      cout << "Intiating invert filter" << endl;
      convert_to_threads(invert);
      break;

    case ADDX:
      cout << "Intiating Plus Mark filter" << endl;
      plus_mark();
      update_real = false;
      break;
    }

    // This part is taken from a stackoverflow question
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Execution Time : " << std::fixed
                           << std::setprecision(2)
                           << duration.count()/1000.0 << " ms" << endl;

    total_time += duration.count()/1000.0;

    start = high_resolution_clock::now();

    if (update_real){
      if (update_real_mirror_y)
        update_original_pixels_col();
      else
        update_original_pixels();
    }
  }

  return;
}

void filter_multithreaded(char *fileBuffer, int bufferSize, char *fileName)
{
  auto parallel_start = high_resolution_clock::now();

  int NUMBER_OF_THREADS_COL_2 = (cols / TH_COL);
  initialize_pixels_threads(NUMBER_OF_THREADS_COL_2,rows, TH_COL , pixels_threads_2);
  
  int NUMBER_OF_THREADS_2 = (rows / TH_ROW);
  initialize_pixels_threads(NUMBER_OF_THREADS_2, TH_ROW, cols, pixels_threads);

  auto start = high_resolution_clock::now();

  getPixlesFromBMP24(bufferSize, rows, cols, fileBuffer);

  update_original_pixels();

  run_filters();

  writeOutBmp24(fileBuffer, "output.bmp", bufferSize);

  auto parallel_end = high_resolution_clock::now();
  auto parallel_duration = duration_cast<microseconds>(parallel_end - parallel_start);
  cout << "Total Execution Time: " << std::fixed<< std::setprecision(2) << total_time << " ms" << endl;
}

// Filters are implemented here
void* mirror_vertical(void* tid)
{
  long id = (long) tid;
	for (int i = 0; i < rows/2; i++) {
		for (int j = 0; j < TH_COL; j++)
      for (int k = 0; k < channels; k++){
          int temp1 = original_pixels[i][id*TH_COL+j][k];
          int temp2 = original_pixels[rows-i-1][id*TH_COL+j][k];
          //Swap temp1 and temp2
          pixels_threads_2[id][i][j][k] = temp2;
          pixels_threads_2[id][rows-i-1][j][k] = temp1;
      }
        
  }
        
}

void* mirror_horizontally(void* tid)
{
  long id = (long) tid;

	 for (int i = 0; i < TH_ROW; i++){
		for (int j = 0; j < cols/2; j++)
      for (int k = 0; k < channels; k++){
          int temp1 = original_pixels[id*TH_ROW+i][j][k];
          int temp2 =  original_pixels[id*TH_ROW+i][cols-j-1][k];
          //Swap temp1 and temp2
          pixels_threads[id][i][j][k] = temp2;
          pixels_threads[id][i][cols-j-1][k] = temp1;
      }
        
  }
        
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
void* median(void *tid)
{
  long id = (long) tid;

  for (int i = 0; i < TH_ROW; i++)
    for (int j = 0; j < cols; j++)
      for (int k = 0; k < channels; k++)
        pixels_threads[id][i][j][k] = find_median(id*TH_ROW+i,j,k);

}

void* mean(void *tid)
{
  long id = (long) tid;

  for (int i = 0; i < TH_ROW; i++)
    for (int j = 0; j < cols; j++)
      for (int k = 0; k < channels; k++)
        pixels_threads[id][i][j][k] = min((original_pixels[id*TH_ROW+i][j][k]*0.4 + avg[k]*0.6), 255.00);
}

void* invert(void *tid)
{
  long id = (long) tid;
  
  for (int i = 0; i < TH_ROW; i++)
    for (int j = 0; j < cols; j++)
      for (int k = 0; k < channels; k++)
        pixels_threads[id][i][j][k] = 255-original_pixels[id*TH_ROW+i][j][k];
}

void plus_mark()
{

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

  filter_multithreaded(fileBuffer, bufferSize, fileName);
  
  return 0;
}