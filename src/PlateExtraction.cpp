#include "PlateExtraction.hpp"

using namespace cvb;
using namespace cv;
using namespace std;
void ConnectedComponents(const Mat mat)
{
	IplImage *im8 = cvCreateImage(cvSize(mat.cols, mat.rows), 8, 1);
	IplImage *im32 = new IplImage(mat);
	cvConvertScale(im32, im8, 1);
	//IplImage* labelImg = new IplImage(mat);
	IplImage* threshy = new IplImage(mat);

	IplImage *labelImg=cvCreateImage(cvSize(mat.cols,mat.rows),IPL_DEPTH_LABEL,1);//Image Variable for blobs
	//IplImage *threshy=cvCreateImage(cvSize(mat.cols,mat.rows),8,1);
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
	cvRenderBlobs(labelImg,blobs,im8,dst);

	namedWindow("label",WINDOW_AUTOSIZE);
	cvShowImage("label",dst);
	cout<<"Number of blobs: "<<result<<endl;
	waitKey();
}