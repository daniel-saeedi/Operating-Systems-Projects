#ifndef _TD_HPP_
#define _TD_HPP_

#include <string>

#pragma pack(1)
#pragma once

#define TOTAL_FILTERS 5
#define MIRROR_X 0
#define MIRROR_Y 1
#define MEDIAN 2
#define INVERT 3
#define PLUS_MARK 4

#define RED 0
#define GREEN 1
#define BLUE 2

const int channels = 3;
const int WHITE[] = {255, 255, 255};
const int TH_ROW = 20;


typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned char*** RGBS;


typedef struct tagBITMAPFILEHEADER
{
  WORD bfType;
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizepixels;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#endif