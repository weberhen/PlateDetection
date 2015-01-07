#include "PlateExtraction.hpp"

using namespace cvb;
using namespace cv;
using namespace std;


void ConnectedComponents(const Mat mat, Mat original,Mat sizeOriginal, int x, int y, int hplate, int wplate)
{
	Mat mat8u;
	mat.convertTo(mat8u,CV_8U);
	Mat blank;
	threshold( mat8u, blank, 1, 1,0 );
	imshow("mix",original.mul(blank));
	
	IplImage *im8 = cvCreateImage(cvSize(mat.cols, mat.rows), 8, 1);
	IplImage *im32 = new IplImage(mat);
	IplImage *img32original = new IplImage(sizeOriginal);
	cvConvertScale(im32, im8, 1);

	IplImage *img8original = cvCreateImage(cvSize(mat.cols, mat.rows), 8, 1);
	cvConvertScale(img32original, img8original, 1);

	//IplImage* threshy = new IplImage(mat);
	IplImage *labelImg=cvCreateImage(cvSize(mat.cols,mat.rows),IPL_DEPTH_LABEL,1);//Image Variable for blobs
	IplImage *dst = cvCreateImage(cvSize(mat.cols,mat.rows),8,3);//Image Variable for blobs

	CvBlobs blobs;
	cvLabel(im8, labelImg, blobs);	

	//printIplImage(labelImg);
	
	//Rendering the blobs
	cvRenderBlobs(labelImg,blobs,img32original,dst);
	  
	Mat matLabelImg(labelImg);
	cvFilterByArea(blobs,minPlateArea,maxPlateArea);
	//cout<<"how many blobs? "<<blobs.size()<<endl;
	
	for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		unsigned int minx = it->second->minx; ///< X min.
	    unsigned int maxx = it->second->maxx; ///< X max.
	    unsigned int miny = it->second->miny; ///< Y min.
	    unsigned int maxy = it->second->maxy; ///< y max.
				
		//
		//finds comulative response along rows
		//
	    Vector<int>cumRows (matLabelImg.rows,0);
	    for(unsigned int j=miny;j<maxy;j++)
	    {
	    	for(unsigned int i=minx;i<maxx;i++)
	    	{
	    		if(matLabelImg.at<unsigned int>(j,i)==it->second->label)
	    			cumRows[j]++;
	    	}
	    }

	    int hthresh = wplate;
	    int rmin=-1;
	    int rmax=-1;
	    Vector<bool> ccRow; //used to find the biggest connected component in a Row
	   
	    //binarizing ccRow based on hthresh
	    for(unsigned int i=0;i<cumRows.size();i++)
	    {
	    	if(cumRows[i]>hthresh)
	    	{
	    		if(rmin==-1)
	    			rmin=i;
	    		rmax=i;
	    		ccRow.push_back(true);
	    	}
	    	else
	    		ccRow.push_back(false);
	    }
	    RowColConnectedComponents(ccRow, &rmin, &rmax);

	    //
		//finds comulative response along columns
		//
	    Vector<int>cumCols (matLabelImg.cols,0);
	    for(unsigned int j=minx;j<maxx;j++)
	    {
	    	for(unsigned int i=miny;i<maxy;i++)
	    	{
	    		if(matLabelImg.at<unsigned int>(i,j)==it->second->label)
	    			cumCols[j]++;
	    	}
	    	
	    }

	    int vthresh = hplate;
	    int cmin=-1;
	    int cmax=-1;
	    Vector<bool> ccCol; //used to find the biggest connected component in a Collumn

	    //binarizing ccCol based on vthresh
	    for(unsigned int i=0;i<cumCols.size();i++)
	    {
	    	if(cumCols[i]>vthresh)
	    	{
	    		if(cmin==-1)
	    			cmin=i;
	    		cmax=i;
	    		ccCol.push_back(true);
	    	}
	    	else
	    		ccCol.push_back(false);
	    }
	    RowColConnectedComponents(ccCol, &cmin, &cmax);

	    int width = cmax - cmin;
		int height = rmax - rmin;
			cv::Rect myROI(cmin, rmin, width , height);
			Mat plate = sizeOriginal(myROI);
		if(width>0&&height>0){
		
			imshow("candidate_plate",plate);
		}

		if(height>hplate&&
		   height<hplate*2.5&&
		   width >wplate&&
		   width <wplate*2.5&&
		   width>height*2.5&&
		   width<height*4.5
		  )
		{

		    Scalar stddev;
			Scalar mean;
			//getting the stdev of the patch
			cv::meanStdDev ( plate, mean, stddev );
			double stddev_pxl = stddev.val[0];
			//cout<<"stddev"<<stddev_pxl<<endl;
			imshow("plate",plate);
			if(stddev_pxl>minStdev){
				timeBetweenPlates=clock();

		    	imshow("final_plate",plate);

		    	//I have to sum with the previews value because it is the x,y relative to the rear of the car, and I will compare with the original image coordinates
		    	algX = cmin + x;
				algY = rmin + (y-sizeOriginal.rows);
				algWidth = width;
				algHeight = height;	
				totalAlgPlates++;
			}
		}
	    
	}
	
	cvReleaseImage(&im8);
	cvReleaseImage(&img8original);
	cvReleaseImage(&labelImg);
	cvReleaseImage(&dst);	

}

void printIplImage(const IplImage* src)
{
    for(int row = 0; row < src->height; row++)
    {
        for(int col = 0; col < src->width; col++)
        {
            printf("%d, ", ((unsigned int*)(src->imageData + src->widthStep*row))[col]);
        }
        printf("\n");
    }
}

void RowColConnectedComponents(Vector<bool>ccVector, int *min, int *max)
{
	int ccRowIndexBegin=0;
    int ccRowIndexEnd=0;
	int ccRowSize=0;
    int ccRowBiggestSize=0;

    for(uint i=0;i<ccVector.size();i++)
    {
    	if(ccVector[i]==true)
    	{
    		ccRowSize++;
    	}
    	else
    	{
    		if(ccRowSize>ccRowBiggestSize)
    		{
    			ccRowBiggestSize=ccRowSize;
    			ccRowIndexBegin=i-ccRowBiggestSize;
    			ccRowIndexEnd=i;
    			ccRowSize=0;
    		}
    	}
    	//cout<<"ccVector: "<<ccVector[i]<<" i: "<<i<<endl;
    }
    *min = ccRowIndexBegin;
    *max = ccRowIndexEnd;
}