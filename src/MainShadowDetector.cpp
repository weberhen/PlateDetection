#include "MainShadowDetector.hpp"

double _rho=2;
double _theta=CV_PI/180;
int _threshold=50;
double _minLineLenght=10;
double _maxLineGap=20;

int main(int argc, char** argv)
{
	Mat dst, source, src_gray, src;

    VideoCapture stream("bento_ipiranga_1410.h264");

    if (!stream.isOpened())
    {
        std::cout << "Stream cannot be opened" << std::endl;
        return -1;
    }

	_rho=atof(argv[2]);
	_threshold=atof(argv[3]);
	_minLineLenght=atof(argv[4]);
	_maxLineGap=atof(argv[5]);
	structAsphaltInfo _structAsphaltInfo;

	int alreadyCalled=0; //limit the asphalt median color calculation to one measures
	stream >> src;
	cv::Rect myROI(0,0,src.cols, 240);
	while(1)
    {
        stream >> src;
        if(src.empty()) 
        {
            std::cout << "Error reading video frame" << endl;
        }
        //imshow("Source", src);
		//create a matrix of the same size  and type as src (for dst)
		dst.create(src.size(),src.type());

		//convert src to gray scale (src_gray)
		cvtColor(src,src_gray,CV_BGR2GRAY);
		if(!alreadyCalled){
			_structAsphaltInfo = FreeDrivingSpaceInfo(src_gray);
			alreadyCalled=1;
		}
//		cout<<"Median is: "<<_structAsphaltInfo.median<<endl;
	
		SearchForShadow(src_gray(myROI),_structAsphaltInfo.median);

        waitKey(20);
    }
    stream.release();
    getchar();


	
	return 0;



}
