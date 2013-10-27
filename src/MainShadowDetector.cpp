#include "MainShadowDetector.hpp"

int main(int argc, char** argv)
{
	Mat dst, src_gray;

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

	return 0;

}
