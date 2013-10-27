#ifndef _IMAGEMANIPULATION
#define _IMAGEMANIPULATION

#define LEFT 	0
#define RIGHT 	1
#define UP		2
#define DOWN	3

#include "ShadowDetector.hpp"
using namespace cv;

Point NeighborPixel(Point pos, int direction, Mat img);


#endif
