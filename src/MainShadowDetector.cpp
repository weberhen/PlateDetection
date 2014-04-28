#include "MainShadowDetector.hpp"
#include<time.h>



ifstream myfile ("carlos_gomes_1410.txt");

int main(int argc, char** argv)
{
	InitializeParameters(argv);
	
	if(manualPlateCapture)
		namedWindow("manualPlateCapture",WINDOW_AUTOSIZE);
	
	if(!debugMode)
	{
		VideoCapture stream(argv[1]);
		
    	if (!stream.isOpened())
    	{
	       	std::cout << "Stream cannot be opened" << std::endl;
    	   	return -1;
		}

		stream >> src;
		
		setMouseCallback("manualPlateCapture",on_mouse, &currentClick );
		while(1)
	    {
	 		if(measureTime)
	 			t = clock();
	    	
			//just part of the video can be used because of the reflection in the windshield
			myROI = cv::Rect(0,0,src.cols, 220);
	       	stream >> src;
	       	if(src.empty()) 
	       	{
		       	std::cout << "Error reading video frame" << endl;
		    }

		    frame++;
		    
		    //setting manually the plates for validation purposes
		    //the "frames" condition is here to allow that the manual plate detection can start at any frame
			if(manualPlateCapture && frame>0)
			{
				cvtColor(src,srcGray,CV_BGR2GRAY);
				imshow("manualPlateCapture",srcGray(myROI));
				waitKey(0);
				insertPlateCoordToFile(frame, platesInFrame);
				platesInFrame.clear();

			}
			else
			{
				//convert src to gray scale (srcGray)
				cvtColor(src,srcGray,CV_BGR2GRAY);
				
				//calculates fds gray median only if 5 seconds have passed without any detected plate
				if((((float)(clock()-timeBetweenPlates))/CLOCKS_PER_SEC)>5 ||(!fdsMedianFirstUse))
				{
					_structAsphaltInfo = FreeDrivingSpaceInfo(srcGray);
					if(_structAsphaltInfo.median<100)
						_structAsphaltInfo.median=130;
					timeBetweenPlates=clock();
					fdsMedianFirstUse=true;
				}
				SearchForShadow(srcGray(myROI),_structAsphaltInfo.median);
				
				if(!onRPI)
					imshow("Recorded Video",srcGray(myROI));
				
				if(takeMetrics)
					float metric = calculateMetric();
				
				algX=0;
				algY=0;
				algWidth=0;
				algHeight=0;

				if(!onRPI)
					waitKey(2);
				if(measureTime)
				{
					t = clock() - t;
					printf("fps: %f.\n",1/((((float)t)/CLOCKS_PER_SEC)-(((float)old_t)/CLOCKS_PER_SEC)));
				}
			}
		}
		
	    myfile.close();	
	    stream.release();
	    getchar();
	}
	else
	{
		//debug code can be put here
	}
	return 0;



}
