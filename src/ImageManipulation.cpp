#include "ImageManipulation.hpp"
#include <fstream>

Point NeighborPixel(Point pos, int direction, Mat img)
{
	switch(direction)
	{
		case LEFT: 
				if(pos.y==0) 				//left border, return the own pixel
					return pos;
				else{ 						//any other pixel in the image
					pos.y--;
					return pos;
				}
		case RIGHT:
				if(pos.y==(img.cols-1)) 	//right border, return the own pixel
					return pos;
				else{						//any other pixel in the image
					pos.y++;
					return pos;
				}
		case UP:
				if(pos.x==0)				//first row, return the own pixel
					return pos;
				else{						//any other pixel in the image
					pos.x--;
					return pos;
				}
		case DOWN:
				if(pos.x==img.rows-1)		//last row, return the own pixel
					return pos;
				else{						//any other pixel in the image
					pos.x++;
					return pos;
				}
	}
	return pos;
}

void on_mouse( int e, int x, int y, int d, void *ptr )
{
	if  ( e == EVENT_LBUTTONDOWN )
	{
		currentClick.x=x;
		currentClick.y=y;
		mouseClicks++;
		
  	
    	if(mouseClicks==2)
    	{
    		cout<<"plate marked at corner "<<previewsClick.x<<" "<<previewsClick.y<<" and corner "<<currentClick.x<<" "<<currentClick.y<<endl;
    		mouseClicks=0;
    		platesInFrame.push_back(previewsClick);
    		platesInFrame.push_back(currentClick);
    	}
    	else
    	{
    		previewsClick.x=x;
    		previewsClick.y=y;	    		
    	}
	}
}

void insertPlateCoordToFile(int frame, Vector<Point> coord)
{

	FILE * pFile=0;
	//pFile = fopen ("plates.txt", "a+");
	//writing the frame number and the number of plates marked at this frame
	fprintf (pFile, "%d %lu\n",frame,coord.size()/2);
	if(coord.size()>1)
	{
		for(unsigned int i=0;i<coord.size()-1;i+=2)
		{
			int x1, x2, y1, y2;
			x1 = coord[i].x;
			y1 = coord[i].y;
			x2 = coord[i+1].x;
			y2 = coord[i+1].y;
			int width = abs(x1-x2);
			int height = abs(y1-y2);
			int xTopLeft = min(x1,x2);
			int yTopLeft = min(y1,y2);
			//cout<<"x1: "<<x1<<" y1: "<<y1<<" x2: "<<x2<<" y2: "<<y2<<endl;
			cout<<" width: "<<width<<" height: "<<height<<" xTopLeft: "<<xTopLeft<<" yTopLeft: "<<yTopLeft<<endl;
			fprintf (pFile, "%d %d %d %d\n",xTopLeft,yTopLeft,width,height);
		}	
	}
		
	fclose (pFile);  
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
int counter=0;
float calculateMetric()
{
	FILE *f = fopen("jaccard_our_video3.txt", "a");
	cout<<counter++<<endl;
	float metric=-1;
	string line;
	if (myfile.is_open())
	{
		if( getline (myfile,line) )
		{
			std::vector<std::string> x = split(line, ' ');
			
			int numberOfPlates=stoi(x[1]);
			vector<float> J;

			for(int i=0;i<numberOfPlates;i++)
			{
				totalRealPlates++;
				cout<<"totalRealPlates: "<<totalRealPlates<<endl;
				getline (myfile,line);
				std::vector<std::string> coordFile = split(line, ' ');
				realX = stoi(coordFile[0]);
				realY = stoi(coordFile[1]);
				realWidth = stoi(coordFile[2]);
				realHeight = stoi(coordFile[3]);

				//cout<<"realX: "<<realX<<" realY: "<<realY<<" realWidth: "<<realWidth<<" realHeight: "<<realHeight<<endl;
				//cout<<"algX: "<<algX<<" algY: "<<algY<<" algWidth: "<<algWidth<<" algHeight: "<<algHeight<<endl;
								
				Rect algRect(algX,algY,algWidth,algHeight);
				Rect realRect(realX,realY,realWidth,realHeight);
				Rect interAB = algRect & realRect;
				Rect unionAB = algRect | realRect;
				//jaccard
				//|A/\B|
				//------
				//|A\/B|
				J.push_back((float)interAB.area()/unionAB.area());
				cout<<J.back()<<endl;
				fprintf(f, "%f\n", J.back());
				

			}
			if(numberOfPlates==0){//não havia placas naquele frame
				if(algWidth!=0){//algoritmo obteve um falso positivo
					J.push_back(-1);
					fprintf(f, "%f\n", J.back());
					cout<<J.back()<<endl;
				}
			}
		}
		else
			exit(1);
	}
	else
	{
		cout<<"could not open file for metric calculation"<<endl;
	}
	fclose(f);
	return metric;
}

void InitializeParameters(char** argv)
{
	
	totalRealPlates=0;
	totalAlgPlates=0;
	gotHolePlate=0;
	meanMetricError=0;
	falsePositives=0;
	falseNegatives=0;
	MinimunIntersection=0;

	frameChangeROI = 		stoi(argv[2]); //MainShadowDetector.. to set specific part of video where the ROI will change it height (4000)
	percentil = 			atof(argv[3]); 
	median = 				atof(argv[4]);
	minSegmentSizeRatio = 	atof(argv[5]); //||((segmentSize>i*1.35) || (segmentSize<i*0.65)))
	maxSegmentSizeRatio = 	atof(argv[6]); //||((segmentSize>i*1.35) || (segmentSize<i*0.65)))
	minStdev = 				stoi(argv[7]);
	minPlateWidthRatio = 	stoi(argv[8]); // 1.2
	minPlateHeight =		stoi(argv[9]); //15
	maxPlateWidth = 		stoi(argv[10]);
	maxPlateHeightRatio = 	stoi(argv[11]); //4
	winy1 = 				stoi(argv[12]); 
	winy2 = 				stoi(argv[13]);
	winy3 = 				stoi(argv[14]);
	winx1 = 				stoi(argv[15]);
	winx2 = 				stoi(argv[16]);
	winx3 = 				stoi(argv[17]);
	minPlateArea = 			stoi(argv[18]);
	maxPlateArea = 			stoi(argv[19]);
}