#include "ShadowDetector.hpp"


Mat src_gray;
Mat dst, detected_edges;

int edgeThreshold = 1;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
String window_name = "Edge Map";
CvPoint pt[4];

/////////////////////////////////////////////////////////////////

void drawSquares2( IplImage* img, CvSeq* squares )
{
    CvSeqReader reader;
    IplImage* cpy = cvCloneImage( img );    
    
    // initialize reader of the sequence
    cvStartReadSeq( squares, &reader, 0 );
    
    int width = 0, height = 0;
    printf("%d squares found\n", squares->total/4);
    // read 4 sequence elements at a time (all vertices of a square)
    for(int i = 0; i < squares->total; i += 4 )
    {
        char* name_with_extension = (char*)malloc(sizeof(char)*60); 
        CvPoint* rect = pt;
        int count = 4;
        int max_x=0, min_x=99999,max_y=0,min_y=99999;
        // read 4 vertices
        memcpy( pt, reader.ptr, squares->elem_size );
        CV_NEXT_SEQ_ELEM( squares->elem_size, reader );
        memcpy( pt + 1, reader.ptr, squares->elem_size );
        CV_NEXT_SEQ_ELEM( squares->elem_size, reader );
        memcpy( pt + 2, reader.ptr, squares->elem_size );
        CV_NEXT_SEQ_ELEM( squares->elem_size, reader );
        memcpy( pt + 3, reader.ptr, squares->elem_size );
        CV_NEXT_SEQ_ELEM( squares->elem_size, reader );
        //if(true)
        {
            //printf("x:%d y:%d\n",pt[0].x,pt[0].y);
            //printf("x:%d y:%d\n",pt[1].x,pt[1].y);
            //printf("x:%d y:%d\n",pt[2].x,pt[2].y);
            //printf("x:%d y:%d\n--------\n",pt[3].x,pt[3].y);
            //printf("parada 12\n");
            for(int j=0;j<4;j++)
            {
                if(pt[j].x > max_x)
                    max_x=pt[j].x;
                if(pt[j].x < min_x)
                    min_x=pt[j].x;
                if(pt[j].y > max_y)
                    max_y=pt[j].y;
                if(pt[j].y < min_y)
                    min_y=pt[j].y;
            }
            width = max_x-min_x;
            height = max_y-min_y;
            printf("width:%d height:%d\n",width,height);

            if((height < width/3) && (height < img->height/2) && (width < img->width/2) && (height < width/2))
            {
                printf("passed\n");
                cvSetImageROI(img, cvRect(min_x,min_y,max_x-min_x,max_y-min_y));
                // cropped image
                IplImage *cropSource = cvCreateImage(cvSize(width,height), img->depth, img->nChannels);
                // copy
                //printf("img - width:%d height:%d \n",img->width, img->height);
                //printf("cropSource - width:%d height:%d \n",cropSource->width, cropSource->height);
                
                namedWindow("GENIOUS",CV_WINDOW_AUTOSIZE);
                cvShowImage("GENIOUS",cropSource);
                
                
            }
        }
        // draw the square as a closed polyline 
        //cvPolyLine( cpy, &rect, &count, 1, 1, CV_RGB(0,255,0), 3, 8,0 );
    }
    
    // show the resultant image
    //cvShowImage("image",cpy );
    cvReleaseImage( &cpy );
}

////////////////////////////////////////////////////////////////



/*
 * \param img the image that will wave one line erased
 * \param segmentSize the size of the segment that will be erased
 * \param i the row where the segment is
 * \param j the col where the segment is
 * Erases an entire segment that is not selected as car shadow
 * \return the image without the segment
 */
Mat EraseLine(Mat img, int segmentSize, int i, int j)
{
	while(segmentSize>=0)
	{
		img.at<unsigned char>(i,j-segmentSize)=0;
		segmentSize--;
	}
	return img;
}

/*
 * \param src_gray image that is receiving the squares that are embracing the cars
 * \param i the row of the bottom right corner of the square
 * \param j the col of the bottom right corner of the square
 * \param segmentSize the size of the car (horizontal)
 * surround the car with a square using square() function from opencv
 * \return the image with the square
 */
Mat SurroundCar(Mat src_gray,int i,int j,int segmentSize)
{
	//segmentSize = SizeOfCar(&src_gray, i, j); 
//	imshow("Merged lines", src_gray);
//	waitKey();
	int height;
	if(i-segmentSize<0)
		height=0;
	else
		height=i-segmentSize;

	rectangle(src_gray,Point(j-segmentSize,height),Point(j,i),Scalar(100,100,100),2,CV_AA);
	segmentSize=0;
	//cout<<"car detected"<<endl;
	return src_gray;
}

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

vector<Vec4i> excludeDuplicateShadows(vector<Vec4i> lines)
{
	for(unsigned int i =0; i < (lines.size()-1);i++)
	{
		Vec4i element;
		if(lines.size()!=0) //if there are no shadows, this could lead to a segfault
			element = lines[i];
		else //this will prevent the next "for" loop to execute
			return lines;
		for(size_t j=i+1;j<lines.size();j++)
		{
			Vec4i list = lines[j];
			//if j of element is similar to j of another (10% threshold)	
			if((element[0]>list[0]*0.9)&&(element[0]<list[0]*1.1))
				if((element[1]>list[1]*0.9)&&(element[1]<list[1]*1.1))
				{	
					lines[i][0]=min(element[0],list[0]);
					lines[i][2]=max(element[2],list[2]);
					lines[i][3]=element[1];
					lines.erase(lines.begin()+j);
				}
		}
		
	}
	//cout<<"ANOTHER ROUND"<<endl;
	for(size_t i =0; i < lines.size();i++)
	{	
		Vec4i element = lines[i]; //one line is compared to the rest of the list
		for(size_t j=i+1;j<lines.size();j++)
		{
			Vec4i list = lines[j]; //here the others lines are selected, one by one
			//if element is similar to another line of the list (10% threshold)	
			if((element[1]>list[1]*0.8)&&(element[1]<list[1]*1.2)){
				//if there are lines closed enough to be merged
				if(((element[0]>list[0])&&(element[0]<list[2]))||
				   ((element[2]>list[0])&&(element[2]<list[2]))||
				   ((list[0]>element[0])&&(list[0]<element[2]))||
				   ((list[2]>element[0])&&(list[2]<element[2])))
				{	
					j--; //to erase the right one
					lines[i][0]=min(element[0],list[0]);
					lines[i][2]=max(element[2],list[2]);
					lines[i][3]=element[1];
					lines.erase(lines.begin()+j+1);
				}
			}
		}
		
	}
	return lines;
}

vector<Vec4i> mergeLines(Mat src)
{
	vector<Vec4i> lines;
	Vec4i element;
	int segmentSize=0;
	int i=src.rows-1;
	bool beginOfNewLine=true;
	Mat dst = Mat::zeros( src.size(), src.type() );
	for(int j=0;j<src.cols;j++)
	{
		while((src.at<unsigned char>(i,j)==0)&&(i>0))//while there are no shadow in the col		
		{
			i--;
		}
		if(i>1) //a shadow was found at "j" column
		{	
			//dst.at<unsigned char>(i,j)=255;
			if(beginOfNewLine)
			{
				beginOfNewLine=false;
				element[0]=j; //saves the x axis of the beginning of the shadow
				element[1]=i; //...and the y axis
			}
			segmentSize++;
		}
		else //the shadow is over. Lets see if it is worth to consider as a car shadow
		{	//if the shadow has a minimum size of 5 pixels
			if((segmentSize>5) && (beginOfNewLine==false))
			{
				element[2]=j;
				element[3]=element[1];
				beginOfNewLine=true;
				lines.push_back(element);
			}
		}
		i=src.rows-1;
	}
	return lines;
}

void SearchForShadow(Mat src,int uBoundary)
{
	Mat bigImg(src);
	// HEREEE namedWindow("small", CV_WINDOW_AUTOSIZE);
	//HEREEE namedWindow("small2", CV_WINDOW_AUTOSIZE);
	Size smallSize(src.cols*0.3,src.rows*0.3);
	int segmentSize=0;
	Mat smallerImg = Mat::zeros( smallSize, src.type());
	Mat dst = Mat::zeros(smallSize,src.type());
	Mat shadows = Mat::zeros(smallSize,src.type());
	resize(src,smallerImg,smallerImg.size(),0,0,INTER_CUBIC);
	resize(src, src, smallerImg.size(),0,0,INTER_CUBIC);
	//imshow("small",src);

	for(int i=smallerImg.rows-1;i>0;i--){
		for(int j=0;j<smallerImg.cols;j++)
		{
			int currentPixel = smallerImg.at<unsigned char>(i, j);
			int downNeighbor = smallerImg.at<unsigned char>(i+1,j);

			if((currentPixel<uBoundary*0.8)&&(downNeighbor>currentPixel))
				dst.at<unsigned char>(i,j)=255;
		}
	}

	for(int i=smallerImg.rows-2;i>1;i--){
		for(int j=1;j<smallerImg.cols-1;j++)
		{
			if(PixelBelongToSegment(dst,i,j))
			{
				segmentSize++;
				shadows.at<unsigned char>(i,j)=255;
			}
			else 
			{
				if(segmentSize<(0.10*smallerImg.cols)
				||((segmentSize>i*1.35) || (segmentSize<i*0.65)))
					EraseLine(shadows, segmentSize, i, j);
				segmentSize=0;
			}
		}
		EraseLine(shadows, segmentSize, i, smallerImg.cols-2);
		segmentSize=0;
	}
	vector<Vec4i> lines;

	lines = mergeLines(shadows);
		
	//imshow("small",shadows);
	//waitKey();		
	lines = excludeDuplicateShadows(lines);
	//for each shadow that was identified, create the Region of Interest (ROI)
	for(int i =0;i<lines.size();i++)
	{
		Vec4i l = lines[i];
		//SurroundCar(row, col, size)
		int x = (l[0])/0.3;
		int y = (l[3]/3)/0.3;
		int width = (l[2]-l[0])/0.3;
		int height = l[3]/0.3-y;
		width=width*0.85; //reduces the width's ROI in 15% 
		x=x+width*0.15;   //reduces the x axis of the ROI in 15% 
		height=height*0.75; //discard the shadow of the vehicule
		//src = SurroundCar(src,l[3],l[2],l[2]-l[0]);
		//cv::Rect(x, y, width, height)
		cv::Rect myROI(x, y, width, height);
		vector<vector<Point> > squares;
		Mat matImg = bigImg(myROI);
		
		//Mat rearVehicule = bigImg(myROI);
		//SOBEL
		 /*Mat grad_x, grad_y;
		Mat abs_grad_x, abs_grad_y;
		int scale = 1;
		int delta = 0;
		int ddepth = CV_16S;
		Mat grad;
		/// Gradient X
		//Scharr( image, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
		Sobel( rearVehicule, grad_x, ddepth, 0, 0, 3, scale, delta, BORDER_DEFAULT );
		convertScaleAbs( grad_x, abs_grad_x );
		namedWindow("ImageSobelGx", CV_WINDOW_AUTOSIZE );
		imshow( "ImageSobelGx", abs_grad_x );
		*/
		//HOUGH
		/*vector<Vec4i> lines;
		Canny(rearVehicule, rearVehicule, 100, 100,5);//thresh, 5);
	    HoughLinesP( rearVehicule, lines, 1, CV_PI/180, 80, 30, 10 );
	    for( size_t i = 0; i < lines.size(); i++ )
	    {
	        line( rearVehicule, Point(lines[i][0], lines[i][1]),
	            Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
	    }
	    namedWindow("ImageSobelGx", CV_WINDOW_AUTOSIZE );
		imshow( "ImageSobelGx", rearVehicule );
		*/
		//findSquares(rearVehicule, squares);
		//drawSquares(rearVehicule, squares);
		//imshow("small", bigImg(myROI));
		IplImage* rearVehicule = new IplImage(bigImg(myROI));
		IplImage *cropSource = cvCreateImage(cvSize(width,height), rearVehicule->depth, rearVehicule->nChannels);
        
        cvCopy(rearVehicule, cropSource, NULL);
        cv::Mat img_mat(rearVehicule);
        cv::Mat img_mat_out(rearVehicule);
        cv::Mat in[] = {img_mat, img_mat, img_mat};
        cv::merge(in, 3, img_mat_out);
        Mat intact = matImg.clone();
        float min_thresh=0.85;
        float max_thresh=1.15;
        //left to right
        for(int i2=0;i2<matImg.rows;i2++)
        {
        	//int corBorda = matImg.at<unsigned char>(i2,0);
        	for(int j=0; j<matImg.cols-1;j++)
        	{
        		int corPixelAtual = matImg.at<unsigned char>(i2,j);
        		int pixelVizinho = matImg.at<unsigned char>(i2,j+1);
        		if((corPixelAtual > pixelVizinho*min_thresh) && (corPixelAtual < pixelVizinho*max_thresh))//&&(matImg.at<unsigned char>(i2,j) < matImg.at<unsigned char>(i2,0)*1.2))
        				matImg.at<unsigned char>(i2,j)=0;
        		else
        			j=matImg.cols-1;
        	}
        }

        //right to left
        for(int i2=0;i2<matImg.rows;i2++)
        {
        	//int corBorda = matImg.at<unsigned char>(i2,matImg.cols-1);
        	for(int j=matImg.cols-1; j>1;j--)
        	{
        		int corPixelAtual = matImg.at<unsigned char>(i2,j);
        		int pixelVizinho = matImg.at<unsigned char>(i2,j-1);
        		if((corPixelAtual > pixelVizinho*min_thresh) && (corPixelAtual < pixelVizinho*max_thresh))//&&(matImg.at<unsigned char>(i2,j) < matImg.at<unsigned char>(i2,0)*1.2))
        				matImg.at<unsigned char>(i2,j)=0;
        		else
        			j=0;
        	}
        }

         //up to down
        for(int j=0; j<matImg.cols;j++)
        {
        	//int corBorda = matImg.at<unsigned char>(0,j);
        	for(int i2=0;i2<matImg.rows-1;i2++)
        	{
        		int corPixelAtual = matImg.at<unsigned char>(i2,j);
        		int pixelVizinho = matImg.at<unsigned char>(i2+1,j);
        		if((corPixelAtual > pixelVizinho*min_thresh) && (corPixelAtual < pixelVizinho*max_thresh))//&&(matImg.at<unsigned char>(i2,j) < matImg.at<unsigned char>(i2,0)*1.2))
        				matImg.at<unsigned char>(i2,j)=0;
        		else
        			i2=matImg.rows-1;
        	}
        }

        //bottom up
        for(int j=0; j<matImg.cols;j++)
        {
        	//int corBorda = matImg.at<unsigned char>(0,j);
        	for(int i2=matImg.rows-1;i2>1;i2--)
        	{
        		int corPixelAtual = matImg.at<unsigned char>(i2,j);
        		int pixelVizinho = matImg.at<unsigned char>(i2-1,j);
        		if((corPixelAtual > pixelVizinho*min_thresh) && (corPixelAtual < pixelVizinho*max_thresh))//&&(matImg.at<unsigned char>(i2,j) < matImg.at<unsigned char>(i2,0)*1.2))
        				matImg.at<unsigned char>(i2,j)=0;
        		else
        			i2=0;
        	}
        }

        //IplImage* ipl_img = new IplImage(img_mat_out);
		//textDetection(ipl_img, 1);
		//cvShowImage("small", rearVehicule);
		findSquares(matImg, squares);
		drawSquares(intact, squares);

		
		//namedWindow("GENIOUS", CV_WINDOW_AUTOSIZE);
		//imshow("GENIOUS",matImg);
		}	
	imshow("small2", bigImg);
	//waitKey();

}

/*
 * \param smallerImg 
 * \param row row of the pixel
 * \param col col of the pixel
 * calculates the size of the car (horizontal) based on the size of the shadow
 * \return the size of the car
 */
int SizeOfCar(Mat *img, int row, int col)
{
	int _sizeOfCar=0;
	int pixel =			(*img).at<unsigned char>(row+1, col);
	int pixelAbove = 	(*img).at<unsigned char>(row+1, col);
	int pixelBelow = 	(*img).at<unsigned char>(row-1, col);
	while(((pixel!=0)||(pixelAbove!=0)||(pixelBelow!=0))&&(col>=0))
	{
		(*img).at<unsigned char>(row+1, col)=0;
		(*img).at<unsigned char>(row, col)=125;
		(*img).at<unsigned char>(row-1, col)=0;
		_sizeOfCar++;
		col--;
	}
	return _sizeOfCar;
}

/*
 * \param dst the image where the pixel is
 * \param i the row of the pixel
 * \param j the col of the pixel
 * Given a pixel's position, it returns if it belongs to a segment or not
 * \return true if it belongs, and false otherwise
 */
bool PixelBelongToSegment(Mat dst, int i, int j)
{
	if(dst.at<unsigned char>(i, j)==255 //8-connectivity
 	&&(dst.at<unsigned char>(i-1, j-1)==255
	|| dst.at<unsigned char>(i-1, j)==255
	|| dst.at<unsigned char>(i-1, j+1)==255
	|| dst.at<unsigned char>(i, j-1)==255
	|| dst.at<unsigned char>(i, j+1)==255
	|| dst.at<unsigned char>(i+1, j-1)==255
	|| dst.at<unsigned char>(i+1, j)==255
	|| dst.at<unsigned char>(i+1, j+1)==255))
		return true;
	else
		return false;
}


