#ifndef PLATEEXTRACTION
#define PLATEEXTRACTION

#include "cvblob.h"
#include "opencv2/opencv.hpp"

using namespace cv;

#include "ShadowDetector.hpp"

void ConnectedComponents(const Mat mat, Mat original, Mat sizeOriginal, int z);
void printIplImage(const IplImage* src);
#endif