#include "PlateExtraction.hpp"

using namespace cvb;
using namespace cv;
using namespace std;
void ConnectedComponents(const Mat mat, Mat original, int z)
{
	namedWindow("plate",WINDOW_AUTOSIZE);
	IplImage *im8 = cvCreateImage(cvSize(mat.cols, mat.rows), 8, 1);
	IplImage *im32 = new IplImage(mat);
	IplImage *img32original = new IplImage(original);
	cvConvertScale(im32, im8, 1);
	
	IplImage *img8original = cvCreateImage(cvSize(mat.cols, mat.rows), 8, 1);
	cvConvertScale(img32original, img8original, 1);

	IplImage* threshy = new IplImage(mat);

	IplImage *labelImg=cvCreateImage(cvSize(mat.cols,mat.rows),IPL_DEPTH_LABEL,1);//Image Variable for blobs
	
	IplImage *dst = cvCreateImage(cvSize(mat.cols,mat.rows),8,3);//Image Variable for blobs

	CvBlobs blobs;
	unsigned int result = cvLabel(im8, labelImg, blobs);	
	//void cvRenderBlobs(const IplImage *imgLabel
	//					, CvBlobs &blobs
	//					, IplImage *imgSource
	//					, IplImage *imgDest
	//					, unsigned short mode=0x000f
	//					, double alpha=1.
	//					);
	//Rendering the blobs
	cvRenderBlobs(labelImg,blobs,img32original,dst);

	cvFilterByArea(blobs,300,100000);
	for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		double moment10 = it->second->m10;
		double moment01 = it->second->m01;
		double area = it->second->area;
		//Variable for holding position
		int x1;
		int y1;
		//Calculating the current position
		x1 = moment10/area;
		y1 = moment01/area;

		unsigned int minx = it->second->minx; ///< X min.
	    unsigned int maxx = it->second->maxx; ///< X max.
	    unsigned int miny = it->second->miny; ///< Y min.
	    unsigned int maxy = it->second->maxy; ///< y max.
		
		//Mapping to the screen coordinates
		//Printing the position information
		//cout<<"X: "<<x1<<" Y: "<<y1<<" minx: "<<minx<<" maxx: "<<maxx<<" miny: "<<miny<<" maxy: "<<maxy<<endl;
		int width = maxx - minx;
		int height = maxy - miny;
		cv::Rect myROI(minx, miny, width , height);
		if((width > height*2.5)){
			imshow("plate",original(myROI));
			cout<<"z: "<<z<<" width: "<<width<<endl;
			//waitKey();
		}
		//cvShowImage("plate",dst);

	}
	//Showing the images
	//cvShowImage("plate",dst);
	
	
}