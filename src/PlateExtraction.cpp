#include "PlateExtraction.hpp"

using namespace cvb;
using namespace cv;
using namespace std;


void ConnectedComponents(const Mat mat, Mat original,Mat sizeOriginal, int z)
{
//	namedWindow("plate",WINDOW_AUTOSIZE);
	IplImage *im8 = cvCreateImage(cvSize(mat.cols, mat.rows), 8, 1);
	IplImage *im32 = new IplImage(mat);
	IplImage *img32original = new IplImage(original);
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
	cvFilterByArea(blobs,300,100000);
	//cout<<"how many blobs? "<<blobs.size()<<endl;
	//namedWindow("labelImg",WINDOW_AUTOSIZE);
	//cvShowImage("im8",labelImg);	
	
	for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		//cout << "Blob #" << it->second->label << ": Area=" << it->second->area << ", Centroid=(" << it->second->centroid.x << ", " << it->second->centroid.y << ")" << endl;
		/*double moment10 = it->second->m10;
		double moment01 = it->second->m01;
		double area = it->second->area;
		//Variable for holding position
		int x1;
		int y1;
		//Calculating the current position
		x1 = moment10/area;
		y1 = moment01/area;*/

		unsigned int minx = it->second->minx; ///< X min.
	    unsigned int maxx = it->second->maxx; ///< X max.
	    unsigned int miny = it->second->miny; ///< Y min.
	    unsigned int maxy = it->second->maxy; ///< y max.
		
		//minx = (sizeOriginal.cols * minx) / 80;
	    //maxx = (sizeOriginal.cols * maxx) / 80;
	    //miny = (sizeOriginal.rows * miny) / original.rows;
	    //maxy = (sizeOriginal.rows * maxy) / original.rows;

	    
		//cout<<"cols: "<<matLabelImg.cols<<" rows: "<<matLabelImg.rows<<" minx: "<<minx<<" maxx: "<<maxx<<endl;
		//cout<<"miny: "<<miny<<" maxy: "<<maxy<<endl;
		//imshow("labelImg",mat);
		//waitKey();
		
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
	    		//printf("cum: %d\n",matLabelImg.at<unsigned int>(j,i))	;
	    	}
	    }

	    int hthresh = 15;
	    int rmin=-1;
	    int rmax=-1;
	    for(unsigned int i=0;i<cumRows.size();i++)
	    {
	    	if(cumRows[i]>hthresh)
	    	{
	    		if(rmin==-1)
	    			rmin=i;
	    		rmax=i;
	    	}
	    }
	    //cout<<"rmin: "<<rmin<<" rmax: "<<rmax<<endl;
	    	
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

	    int vthresh = hthresh/3;
	    int cmin=-1;
	    int cmax=-1;
	    for(unsigned int i=0;i<cumCols.size();i++)
	    {
	    	if(cumCols[i]>vthresh)
	    	{
	    		if(cmin==-1)
	    			cmin=i;
	    		cmax=i;
	    	}
	    }
	    //cout<<"cmin: "<<cmin<<" cmax: "<<cmax<<endl;
	    

	    int width = cmax - cmin;
		int height = rmax - rmin;


		if((width > height*2.5) &&
		   (width < height*4) &&
		   (width>25)&&(height>15)&&
		   (width<matLabelImg.cols/2))
		{
			cv::Rect myROI(cmin, rmin, width , height);
		    Mat plate = sizeOriginal(myROI);
		    Scalar stddev;
			Scalar mean;
			//getting the stdev of the patch
			cv::meanStdDev ( plate, mean, stddev );
			double stddev_pxl = stddev.val[0];
			//cout<<"stddev"<<stddev_pxl<<endl;
			if(stddev_pxl>20){
				timeBetweenPlates=clock();
		    	imshow("final_plate",plate);	
			}
		    
		   // waitKey();	
		}
	    
		/*
		//correcting te values (because of the reduction to save time)
	    minx = (sizeOriginal.cols * minx) / 80;
	    maxx = (sizeOriginal.cols * maxx) / 80;
	    miny = (sizeOriginal.rows * miny) / original.rows;
	    maxy = (sizeOriginal.rows * maxy) / original.rows;

		int width = maxx - minx;
		int height = maxy - miny;
		cv::Rect myROI(minx, miny, width , height);
		float tg11 = 0.1909; //tangent of 11o
		float opp = width/2;

		Mat plate = sizeOriginal(myROI);
		Scalar stddev;
		Scalar mean;
		//getting the stdev of the patch
		cv::meanStdDev ( plate, mean, stddev );
		//uchar       mean_pxl = mean.val[0];
		double       stddev_pxl = stddev.val[0];

		namedWindow("final_plate",WINDOW_AUTOSIZE);
		if((width > height*2.5) &&
		   (width < height*4) &&
			(((opp/(float)z) < (tg11*3))&&
			(((opp/(float)z) > (tg11*0.1)))) &&
			stddev_pxl>20
		)
		{
			//imwrite("final.png",plate);
			imshow("final_plate",plate);
		}
		*/
	}

	
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
