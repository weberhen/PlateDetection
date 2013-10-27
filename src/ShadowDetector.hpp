#ifndef _SHADOWDETECTOR
#define _SHADOWDETECTOR

#include "MainShadowDetector.hpp"
#include "opencv2/opencv.hpp"
#include "MathFunctions.hpp"
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

Mat CannyThreshold(Mat src, int lowThreshold);
Mat FreeDrivingSpace(Mat dst, Mat src,structAsphaltInfo *_structAsphaltInfo);
structAsphaltInfo FreeDrivingSpaceInfo(Mat src_gray);


#endif
