#ifndef _SHADOWDETECTOR
#define _SHADOWDETECTOR


#include "opencv2/opencv.hpp"
#include "SquareDetection.hpp"
#include "MathFunctions.hpp"
#include "ImageManipulation.hpp"
#include "PlateExtraction.hpp"
#include "Histogram.hpp"
//#include "RobustMatcher.cpp"
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

extern clock_t timeBetweenPlates;

extern int mouseClicks;
extern int frame;
extern Point currentClick, previewsClick;
extern Vector<Point> platesInFrame;

extern int algX;
extern int algY;
extern int algWidth;
extern int algHeight;
extern ifstream myfile;

extern int totalRealPlates;
extern int totalAlgPlates;
extern int falsePositives;
extern int falseNegatives;
extern int gotHolePlate;
extern float meanMetricError;

//////////////////////////////////////////////////////////////
//parameters to be seted by the user (to reach the best performance)
extern int frameChangeROI;
extern float percentil;
extern float median;
extern float minSegmentSizeRatio;
extern float maxSegmentSizeRatio;
extern int minStdev;
//int width = cmax - cmin;
//int height = rmax - rmin;
extern int minPlateWidthRatio;
extern int minPlateHeight;
extern int maxPlateWidth;
extern int maxPlateHeightRatio;
//int winy_size[3] = {1,2,3};
//int winx_size[3] = {1,1,2};
extern int winy1;
extern int winy2;
extern int winy3;
extern int winx1;
extern int winx2;
extern int winx3;


//////////////////////////////////////////////////////////////

Mat EraseLine(Mat img, int segmentSize, int i, int j);
Mat SurroundCar(Mat src,int i,int j,int segmentSize);
Mat CannyThreshold(Mat src, int lowThreshold);
Mat FreeDrivingSpace(Mat dst, Mat src,structAsphaltInfo *_structAsphaltInfo);
structAsphaltInfo FreeDrivingSpaceInfo(Mat src_gray);
vector<Vec4i> excludeDuplicateShadows(vector<Vec4i> lines);
vector<Vec4i> mergeLines(Mat src);
Mat TransitionToShadow(Mat input, int uBoundary);
Mat ExludeFalseShadowPixels(Mat input, Size size);
void CreateROIOfShadow(vector<Vec4i> lines, Mat input, float reductionFactor);
Mat localSTD(Mat input, Mat xi, Mat xi2, int ind);
void IsolatePlate(Mat input,int z, int x, int y);
void SearchForShadow(Mat src,int uBoundary);
int SizeOfCar(Mat *smallerImg, int y, int x);
bool PixelBelongToSegment(Mat dst, int i, int j);


#endif
