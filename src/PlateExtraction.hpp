#ifndef PLATEEXTRACTION
#define PLATEEXTRACTION

#include "cvblob.h"
#include "opencv2/opencv.hpp"

using namespace cv;

void ConnectedComponents(const Mat mat, Mat original, int z);

#endif