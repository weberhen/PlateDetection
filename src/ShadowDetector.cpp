#include "ShadowDetector.hpp"

Mat src_gray;
Mat dst, detected_edges;

int edgeThreshold = 1;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
String window_name = "Edge Map";

Mat CannyThreshold(Mat src, int lowThreshold)
{
	//reduce noise with a kernel 3x3
	blur(src, detected_edges, Size(3,3));
	//canny detector
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);
	//canny will be a mask, so we create a black image here
	dst = Scalar::all(0);
	//then we paste over the canny output
	src.copyTo(dst, detected_edges);
	return dst;
}

/*
* \param dst image with canny applied
* \param src original image from where the original pixel colors are taken from
*
*/
Mat FreeDrivingSpace(Mat dst, Mat src,structAsphaltInfo *_structAsphaltInfo)
{
	int width = dst.cols;
	int height = dst.rows;
	Mat new_dst = Mat::zeros( dst.size(), dst.type() );
	new_dst = Scalar::all(-1);
	int arrayMeanAsphaltColor[width*height];
	int indexMAC = 0; //size of arrayMeanAsphaltColor;
	for(int j=0.26*width; j<width-(0.26*width); j++)
	{
		for(int i=height-1; i>0; i--)
		{
			if(dst.at<unsigned char>(i, j)!=0)
			{
				new_dst.at<unsigned char>(i, j) = 255;
				i=0;
			}
			else
			{
				new_dst.at<unsigned char>(i, j) = src.at<unsigned char>(i, j*src.channels());
				arrayMeanAsphaltColor[indexMAC] = src.at<unsigned char>(i, j*src.channels());
				indexMAC++;
			}
		}
	}
	std::vector<int> v(arrayMeanAsphaltColor, arrayMeanAsphaltColor + indexMAC);
	sort(v.begin(),v.end());

	(*_structAsphaltInfo).sd=Deviation(v,Average(v));
	(*_structAsphaltInfo).median=v.at(v.size()/2);
	(*_structAsphaltInfo).average=(int)Average(v);
	(*_structAsphaltInfo).porcentageAsphalt=100*indexMAC/(width*height);

	return new_dst;
}

structAsphaltInfo FreeDrivingSpaceInfo(Mat src_gray)
{
	structAsphaltInfo _structAsphaltInfo;
	_structAsphaltInfo.porcentageAsphalt=0;
	Mat dst;
	int lowThreshold=20;
 	while(_structAsphaltInfo.porcentageAsphalt<20 && lowThreshold <= 100)
    {
        //apply canny to the original image
        dst = CannyThreshold(src_gray, lowThreshold);
        //search for the free driving space
        FreeDrivingSpace(dst, src_gray, &_structAsphaltInfo);
        lowThreshold+=20;
    }
	return _structAsphaltInfo;
}

void SearchForShadow(Mat src,int uBoundary)
{
	Size smallSize(src.cols*0.3,src.rows*0.3);
	int segmentSize=0;
	Mat smallerImg = Mat::zeros( smallSize, src.type());
	Mat dst = Mat::zeros(smallSize,src.type());
	Mat shadows = Mat::zeros(smallSize,src.type());
	resize(src,smallerImg,smallerImg.size(),0,0,INTER_CUBIC);
	for(int i=smallerImg.rows-1;i>0;i--){
		for(int j=0;j<smallerImg.cols;j++)
		{
			Point downNeighbor = NeighborPixel(Point(i,j),DOWN,smallerImg);
			if((smallerImg.at<unsigned char>(i, j)<uBoundary*0.8)
				&&(smallerImg.at<unsigned char>(downNeighbor.x, downNeighbor.y)>smallerImg.at<unsigned char>(i, j)))
				dst.at<unsigned char>(i,j)=255;
				
		}
	}
	
	for(int i=smallerImg.rows-2;i>1;i--){
		for(int j=1;j<smallerImg.cols-1;j++)
		{
			if(dst.at<unsigned char>(i, j)==255 //8-connectivity
				&&(dst.at<unsigned char>(i-1, j-1)==255
				|| dst.at<unsigned char>(i-1, j)==255
				|| dst.at<unsigned char>(i-1, j+1)==255
				|| dst.at<unsigned char>(i, j-1)==255
				|| dst.at<unsigned char>(i, j+1)==255
				|| dst.at<unsigned char>(i+1, j-1)==255
				|| dst.at<unsigned char>(i+1, j)==255
				|| dst.at<unsigned char>(i+1, j+1)==255)
			)
			{
				segmentSize++;
				shadows.at<unsigned char>(i,j)=125;
			}
			else 
			{
				if(segmentSize<30)
				{
					while(segmentSize>=0)
					{
						shadows.at<unsigned char>(i,j-segmentSize-1)=0;
						segmentSize--;
					}
				}
				else
					segmentSize=0;
			}
			if(j==smallerImg.cols-2)
			{
				while(segmentSize>=0)
				{
					shadows.at<unsigned char>(i,j-segmentSize-1)=0;
					segmentSize--;
				}
			}
		}
		segmentSize=0;
	}
	namedWindow("small", CV_WINDOW_AUTOSIZE);
	imshow("small", shadows);
	waitKey();	
	
}


