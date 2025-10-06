#ifndef LIB_CV_H
#define LIB_CV_H

#include <ctime>
#include <direct.h>
#include <iostream>
#include <opencv.hpp>
#include <opencv2/opencv.hpp>
#include <windows.h>

#include "log.h"

using namespace cv;

#define DEFAULT_MAXN_MIX 20
#define DEFAULT_MIX_OFFSET 1
#define DEFAULT_OFFSET 1

#define DEFAULT_MIX_MODE 0
#define COCKROACH_MODE 1
#define TANK_MODE 2

#define OUTPUT_JPG 0
#define OUTPUT_PNG 1
#define OUTPUT_TOT 2

#define DIV 30

#define ull unsigned long long
#define ll long long

//==========================================
/* tank */
extern vector<int> compression_params;
extern int div_rgb;

/* copy_modify */
extern int MAXN_MIX;
extern int MIX_OFFSET;
extern int R_OffsetFactor;
extern int G_OffsetFactor;
extern int B_OffsetFactor;
extern int berlinSize;
extern int berlinEnable;

extern int MIX_MODE;

extern int angle;
extern int R_CODE[1000];
#define ROTATE_ANGLE R_CODE[angle]

extern int output_mode;
//==========================================
/*
 * tank
 */
int rangeConvert(int src, int hi, int lo);
int div2(int src, int lo);
int halfRange(int hi, int lo);
void init();
Mat ColorScaleUP(Mat img);
Mat ColorScaleDOWN(Mat img);
void ColorScaleConvert(Mat srcA, Mat srcB, Mat &dstA, Mat &dstB);
Mat invert(Mat img);
Mat MatPlus(Mat A, Mat B);
Mat divide(Mat B, Mat O);
Mat AlphaOpt(Mat img, Mat orc);
Mat GrayScale(Mat img);
Mat getMirageTank(Mat src);
//==========================================
/*
 * copy_modify
 */
bool Hbitmap2Mat(HBITMAP &hBmp, Mat &mat);
bool Mat2HBitmap(HBITMAP &hBmp, Mat &mat);
uchar pixelOffset(uchar origin, int offsetFactor);
void ImageMix(Mat &mat);
void getImgFormClipboard(HWND hwnd);
//==========================================
#endif