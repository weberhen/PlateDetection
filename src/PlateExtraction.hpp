#ifndef PLATEEXTRACTION
#define PLATEEXTRACTION

#include "cvblob.h"
#include "opencv2/opencv.hpp"

using namespace cv;

#include "ShadowDetector.hpp"

void ConnectedComponents(const Mat mat, Mat original,Mat sizeOriginal, int x, int y,int hplate, int wplate);
void printIplImage(const IplImage* src);
void RowColConnectedComponents(Vector<bool>ccVector, int *min, int *max);
#endif