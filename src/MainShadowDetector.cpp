#include "MainShadowDetector.hpp"

int main(int argc, char** argv)
{
	Mat dst, src_gray;

	Mat src = imread(argv[1]);
	if(!src.data)
		return -1;
	//create a matrix of the same size  and type as src (for dst)
	dst.create(src.size(),src.type());
	//convert src to gray scale (src_gray)
	cvtColor(src,src_gray,CV_BGR2GRAY);
	//create a window
	namedWindow("Canny", CV_WINDOW_AUTOSIZE);
	//apply canny and return to src
	for(int i=20;i<100;i+=20)
	{
		dst = CannyThreshold(src_gray, i);
		imshow("Canny", dst);
		waitKey(0);
		src = FreeDrivingSpace(dst, src_gray);
	        imshow("Result", src);
		waitKey(0);
	}
	return 0;

}
