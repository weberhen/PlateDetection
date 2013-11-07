#ifndef _SHADOWDETECTOR
#define _SHADOWDETECTOR

#include "TextDetection.hpp"
#include "opencv2/opencv.hpp"
#include "MathFunctions.hpp"
#include "ImageManipulation.hpp"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

typedef struct asphaltInfo{
	int average;
	int median;
	int sd;
	int porcentageAsphalt;
} structAsphaltInfo;


Mat EraseLine(Mat img, int segmentSize, int i, int j);
Mat SurroundCar(Mat src,int i,int j,int segmentSize);
Mat CannyThreshold(Mat src, int lowThreshold);
Mat FreeDrivingSpace(Mat dst, Mat src,structAsphaltInfo *_structAsphaltInfo);
structAsphaltInfo FreeDrivingSpaceInfo(Mat src_gray);
vector<Vec4i> excludeDuplicateShadows(vector<Vec4i> lines);
vector<Vec4i> mergeLines(Mat src);
void SearchForShadow(Mat src,int uBoundary);
int SizeOfCar(Mat *smallerImg, int y, int x);
bool PixelBelongToSegment(Mat dst, int i, int j);


#endif
