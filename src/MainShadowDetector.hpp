#ifndef _MAINSHADOWDETECTOR
#define _MAINSHADOWDETECTOR

#include "ShadowDetector.hpp"

clock_t timeBetweenPlates = 0;

int mouseClicks=0;
int frame=0;
Point currentClick, previewsClick;
Vector<Point> platesInFrame;
Mat srcGray, src;

//FLAGS
bool debugMode= 		false;
bool manualPlateCapture=false;
bool measureTime= 		false;
bool fdsMedianFirstUse=	false;
bool takeMetrics=		false;
bool onRPI=				false;

structAsphaltInfo _structAsphaltInfo;
clock_t t, old_t = 0;
cv::Rect myROI;

//////////////////////////////////////////////////////////////
//parameters to be set by the user (to reach the best performance)
 int frameChangeROI;
 float percentil;
 float median;
 float minSegmentSizeRatio;
 float maxSegmentSizeRatio;
 int minStdev;
//int width = cmax - cmin;
//int height = rmax - rmin;
 int minPlateWidthRatio;
 int minPlateHeight;
 int maxPlateWidth;
 int maxPlateHeightRatio;
//int winy_size[3] = {1,2,3};
//int winx_size[3] = {1,1,2};
 int winy1;
 int winy2;
 int winy3;
 int winx1;
 int winx2;
 int winx3;
 int minPlateArea;
 int maxPlateArea;

//////////////////////////////////////////////////////////////

int algX;
int algY;
int algWidth;
int algHeight;

int totalRealPlates;
int totalAlgPlates;
int falsePositives;
int falseNegatives;
int gotHolePlate;
float meanMetricError;
int MinimunIntersection;

int realX,realY,realWidth,realHeight;

#endif
