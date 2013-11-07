#ifndef SQUARE_DETECTION
#define SQUARE_DETECTION

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <math.h>
#include <string.h>

using namespace cv;
using namespace std;



double angle( Point pt1, Point pt2, Point pt0 );
void findSquares( const Mat& image, vector<vector<Point> >& squares );
void drawSquares( Mat& image, const vector<vector<Point> >& squares );




#endif