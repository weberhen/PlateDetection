#include "MainShadowDetector.hpp"
#include<time.h>

int main(int argc, char** argv)
{
	Mat dst, source, src_gray, src;

	//Start and end times
	time_t start,end;

	//Start the clock
	time(&start);
	int counter=0;

    VideoCapture stream("bento_ipiranga_1410.h264");

    if (!stream.isOpened())
    {
        std::cout << "Stream cannot be opened" << std::endl;
        return -1;
    }

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
	//Stop the clock and show FPS
	time(&end);
	++counter;
	double sec=difftime(end,start);
	double fps=counter/sec;
	printf("\n time: %lf",sec);
        waitKey(20);
    }
    stream.release();
    getchar();


	
	return 0;



}
