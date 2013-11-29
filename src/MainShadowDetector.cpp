#include "MainShadowDetector.hpp"
#include<time.h>

clock_t timeBetweenPlates = 0;

int mouseClicks=0;
int frame=0;
Point currentClick, previewsClick;
Vector<Point> platesInFrame;

int algX;
int algY;
int algWidth;
int algHeight;
ifstream myfile ("plates.txt");

int main(int argc, char** argv)
{

	Mat src_gray, src;
	bool debugMode = false;
	bool manualPlateCapture=false;
	
	structAsphaltInfo _structAsphaltInfo;
	namedWindow("sss",WINDOW_AUTOSIZE);
	//clock_t t, old_t = 0;
	if(!debugMode)
	{
		 VideoCapture stream("bento_ipiranga_1410.h264");
		 //VideoCapture stream("cristiano_fischer_1010.h264");
		 //VideoCapture stream("carlos_gomes_1410.h264");
		 
    	if (!stream.isOpened())
    	{
	       	std::cout << "Stream cannot be opened" << std::endl;
    	   	return -1;
		}

		stream >> src;
		cv::Rect myROI(0,0,src.cols, 220);
		bool alreadyCalled = false;
		setMouseCallback("sss",on_mouse, &currentClick );
		while(1)
	    	{
	 		//t = clock();

	       	stream >> src;
	       	frame++;
	       	if(src.empty()) 
	       	{
		       	std::cout << "Error reading video frame" << endl;
		    }
		    //setting manually the plates for validation purposes
			if(manualPlateCapture && frame>0)
			{
				cvtColor(src,src_gray,CV_BGR2GRAY);
				imshow("sss",src_gray(myROI));
				waitKey(0);
				insertPlateCoordToFile(frame, platesInFrame);
				platesInFrame.clear();

			}
			else
			{
				//convert src to gray scale (src_gray)
				cvtColor(src,src_gray,CV_BGR2GRAY);
				if((((float)(clock()-timeBetweenPlates))/CLOCKS_PER_SEC)>5 ||(!alreadyCalled))
				//if(!alreadyCalled)
				{
					//cout<<"too long"<<endl;
					_structAsphaltInfo = FreeDrivingSpaceInfo(src_gray);
					if(_structAsphaltInfo.median<100)
						_structAsphaltInfo.median=100;
					timeBetweenPlates=clock();
					alreadyCalled=true;
					//cout<<"median is now "<<_structAsphaltInfo.median<<endl;
				}
				SearchForShadow(src_gray(myROI),_structAsphaltInfo.median);
				
				imshow("sss",src_gray(myROI));
				calculateMetric();
				//cout<<algX<<" "<<algY<<" "<<algWidth<<" "<<algHeight<<endl;
				algX=0;
				algY=0;
				algWidth=0;
				algHeight=0;

				waitKey(2);
				//to measure the fps, just uncomment the following lines
				//t = clock() - t;
				//printf("fps: %f.\n",1/((((float)t)/CLOCKS_PER_SEC)-(((float)old_t)/CLOCKS_PER_SEC)));
			}
		}
		
	    myfile.close();	
	    stream.release();
	    getchar();
	}
	else
	{

		Mat inter = imread("ROI.png",1);
        cvtColor(inter,inter,CV_BGR2GRAY);
        IsolatePlate(inter,0, 0,0);
       	//imwrite("BIG_preta.png",inter);
       	return 0;

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
