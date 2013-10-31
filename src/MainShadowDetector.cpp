#include "MainShadowDetector.hpp"

double _rho=2;
double _theta=CV_PI/180;
int _threshold=50;
double _minLineLenght=10;
double _maxLineGap=20;

int main(int argc, char** argv)
{
	Mat dst, src_gray;

	_rho=atof(argv[2]);
	_threshold=atof(argv[3]);
	_minLineLenght=atof(argv[4]);
	_maxLineGap=atof(argv[5]);

	//open image
	Mat src = imread(argv[1]);
	if(!src.data)
		return -1;

	//create a matrix of the same size  and type as src (for dst)
	dst.create(src.size(),src.type());

	//convert src to gray scale (src_gray)
	cvtColor(src,src_gray,CV_BGR2GRAY);

	structAsphaltInfo _structAsphaltInfo = FreeDrivingSpaceInfo(src_gray);
	cout<<"Median is: "<<_structAsphaltInfo.median<<endl;
	
	SearchForShadow(src_gray,_structAsphaltInfo.median);

	return 0;

}
