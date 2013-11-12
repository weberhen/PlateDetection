#include "MainShadowDetector.hpp"
#include<time.h>

int main(int argc, char** argv)
{
	Mat src_gray, src;
	bool debugMode = false;
	structAsphaltInfo _structAsphaltInfo;

	if(!debugMode)
	{
		 VideoCapture stream("bento_ipiranga_1410.h264");

	    if (!stream.isOpened())
	    {
	        std::cout << "Stream cannot be opened" << std::endl;
	        return -1;
	    }

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

			//convert src to gray scale (src_gray)
			cvtColor(src,src_gray,CV_BGR2GRAY);
			if(!alreadyCalled){
				_structAsphaltInfo = FreeDrivingSpaceInfo(src_gray);
				alreadyCalled=1;
			}
			SearchForShadow(src_gray(myROI),_structAsphaltInfo.median);
	        waitKey(20);
	    }
	    stream.release();
	    getchar();
	}
	else
	{
		namedWindow("Small3",WINDOW_AUTOSIZE);
		src = imread("capturaModificada.png",1);
		cvtColor(src,src_gray,CV_BGR2GRAY);
		_structAsphaltInfo = FreeDrivingSpaceInfo(src_gray);
		Mat transitionToShadow = TransitionToShadow(src_gray, _structAsphaltInfo.median);
		imwrite("TransitionToShadow.png",transitionToShadow);
		Size size(src.cols,src.rows);
		Mat exludeFalseShadowPixels = ExludeFalseShadowPixels(transitionToShadow, size);
		imwrite("ExludeFalseShadowPixels.png",exludeFalseShadowPixels);
		vector<Vec4i> lines = mergeLines(exludeFalseShadowPixels);
		lines = excludeDuplicateShadows(lines);
		Mat original_gray;
		//PS: to save picture from ROI, change drawSquares(matImg, squares); to drawSquares(input, squares); at line 357 of ShadowDetector.cpp
		CreateROIOfShadow(lines, src_gray,1);
		imwrite("createROIOfShadow.png",src_gray);

	}

	
	return 0;



}
