#include "ShadowDetector.hpp"
#include <stdlib.h>

Mat src_gray;
Mat dst, detected_edges;


using namespace std;

int edgeThreshold = 1;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
String window_name = "Edge Map";
CvPoint pt[4];


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

Mat TransitionToShadow(Mat input, int uBoundary)
{

	Mat output = Mat::zeros(input.size(),input.type());
	for(int i=input.rows-1;i>0;i--){
		for(int j=0;j<input.cols;j++)
		{
			int currentPixel = input.at<unsigned char>(i, j);
			int downNeighbor = input.at<unsigned char>(i+1,j);

			if((currentPixel<uBoundary*0.8)&&(downNeighbor>currentPixel))
				output.at<unsigned char>(i,j)=255;
		}
	}
	return output;
}

Mat ExludeFalseShadowPixels(Mat input, Size size)
{
	
	Mat shadows = Mat::zeros(size,input.type());
	int segmentSize=0;
	for(int i=input.rows-2;i>1;i--){
		for(int j=1;j<input.cols-1;j++)
		{
			if(PixelBelongToSegment(input,i,j))
			{
				segmentSize++;
				shadows.at<unsigned char>(i,j)=255;
			}
			else 
			{
				if(segmentSize<(0.10*input.cols)
				||((segmentSize>i*1.35) || (segmentSize<i*0.65)))
					EraseLine(shadows, segmentSize, i, j);
				segmentSize=0;
			}
		}
		EraseLine(shadows, segmentSize, i, input.cols-2);
		segmentSize=0;
	}
	return shadows;
}

void IsolatePlate(Mat input,int z)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	Mat original = input.clone();
	namedWindow("gradient", WINDOW_AUTOSIZE);
	Mat abs_grad_x;
	input.convertTo(input, CV_32F);
	int sz[3] = {input.cols,input.rows,3};
    Mat x;

	Sobel( input, x, CV_32F, 1, 0);

	//convertScaleAbs(x,abs_grad_x);

	// x = gx; % evaluates horizontal details

	Mat xi,xi2;
	cv::integral(x, xi, xi2, CV_64F);
	//abs_grad_x É DO TIPO UNSIGNED CHAR
	//std::cout<<input<<endl;
	//std::cout<<"x end"<<endl;
	//imshow("gradient",x);
	//waitKey();
	//cv::Size s = x.size();
	//int rows = s.height;
	//int cols = s.width;
	//std::cout << x.depth() << " "<<x.type() << " " << rows << " " << cols << " " << std::endl;
	//std::cout << input.depth()<<" "<<input.type() << " " << input.rows << " " << input.cols << " " << std::endl;
	
	//imshow("gradient",xi);
	//std::cout<<x<<endl;
	//waitKey();
	//for(int i2=0;i2<input.rows;i2++){
	//	for(int j2=0;j2<input.cols;j2++)	
	// 		printf("%d %d -- %f\n",i2,j2,x.at<float>(i2,j2));
	// 		waitKey();
	//}
	//xi É DO TIPO DOUBLE
	
	//////////////////////////////////////////////DONE///////////////////////////////////////////////////////
	//std::cout<<xi<<endl;	
	int winy_size[3] = {1,2,3};
	int winx_size[3] = {1,2,3};

	int scols = input.cols;
	int srows = input.rows;
	
    Mat st(3,sz, CV_64FC3, Scalar::all(0));
    for(int ind=0;ind<3;ind++)
    {
    	int winx = winx_size[ind];
	    int winy = winy_size[ind];
	    
	    double Npix = (2*winx+1)*(2*winy+1);
    	for(int i=1+winx;i<srows-winy-1;i++)
    	{
    		for(int j=winy;j<scols-winx-1;j++)
    		{
    			double ex = 	xi.at<double>(i + winx + 1,j + winy + 1)
    						+ 	xi.at<double>(i - winx, j - winy)
    						-  	xi.at<double>(i - winx,j + winy + 1)
    						-	xi.at<double>(i + winx + 1,j - winy);
            	
    			double ex2 = 	xi2.at<double>(i + winx + 1,j + winy + 1)
    						+ 	xi2.at<double>(i - winx, j - winy)
    						-  	xi2.at<double>(i - winx,j + winy + 1)
    						-	xi2.at<double>(i + winx + 1,j - winy);
    			st.at<Vec3f>(i,j)[ind]=sqrt(((Npix*ex2 - ex*ex)/Npix) < 1e-12 ? 0 : (Npix*ex2 - ex*ex)/Npix);
    		}
    		//waitKey();
    	}
    }
    Mat ss(srows, scols, CV_32F);
	for(int i=0;i<srows;i++)
	{
		for(int j=0;j<scols;j++)
		{
		   	ss.at<float>(i,j) = (st.at<Vec3f>(i,j)[0] + st.at<Vec3f>(i,j)[1] + st.at<Vec3f>(i,j)[2])/3;
		}
	}
	
	Mat so = ss.clone();
	cv::sort(so, so, CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
	float T = 0.5*(so.at<float>(so.rows*0.5,so.cols*0.5)+(so.at<float>(so.rows*0.9,so.cols*0.9)));
	//printf("WOW %f\n", T);
	//waitKey();

	for(int i=0;i<srows;i++){
		for(int j=0;j<scols;j++)
			if(ss.at<float>(i,j)<T)
				input.at<float>(i,j)=0.;
			
	}
	//std::cout<<ss<<endl;

	ConnectedComponents(input, original, z);
	    
}

void CreateROIOfShadow(vector<Vec4i> lines, Mat input, float reductionFactor)
{
	Mat intact;
	for(unsigned int i =0;i<lines.size();i++)
	{
		Vec4i l = lines[i];
		int x = (l[0])/reductionFactor;
		int y = (l[3]/3)/reductionFactor;
		int width = (l[2]-l[0])/reductionFactor;
		int height = l[3]/reductionFactor-y;
		width=width*0.85; //reduces the width's ROI in 15% 
		x=x+width*0.15;   //reduces the x axis of the ROI in 15% 
		height=height*0.75; //discard the shadow of the vehicule
		//cout<<"x: "<<x<<"y: "<<y<<"width: "<<width<<"height: "<<height<<endl;
		cv::Rect myROI(x, y, width, height);
		
		vector<vector<Point> > squares;
		
		Mat matImg = input(myROI);
		Mat original = matImg.clone();
        intact = matImg.clone();
        //////////////////////////////////////////////////////////////////////
       /* Mat img1 = imread("plate.png", CV_LOAD_IMAGE_GRAYSCALE);

        float reductionFactor = 2;
		Size smallSize(matImg.cols*reductionFactor,matImg.rows*reductionFactor);
		Mat smallerImg = Mat::zeros( smallSize, matImg.type());
		//make a copy of src to smallerImg reduced to 30% of the original size
		resize(matImg,matImg,smallerImg.size(),0,0,INTER_CUBIC);

        // detecting keypoints
	    SurfFeatureDetector detector(400);
	    vector<KeyPoint> keypoints1, keypoints2;
	    detector.detect(img1, keypoints1);
	    detector.detect(matImg, keypoints2);

	    // computing descriptors
	    SurfDescriptorExtractor extractor;
	    Mat descriptors1, descriptors2;
	    extractor.compute(img1, keypoints1, descriptors1);
	    extractor.compute(matImg, keypoints2, descriptors2);

	    // matching descriptors
	    BruteForceMatcher<L2<float> > matcher;
	    vector<DMatch> matches;
	    matcher.match(descriptors1, descriptors2, matches);

	    // drawing the results
	    namedWindow("matches", 1);
	    Mat img_matches;
	    drawMatches(img1, keypoints1, matImg, keypoints2, matches, img_matches);
	    imshow("matches", img_matches);
		*/
        //////////////////////////////////////////////////////////////////////
        int z = y + height;
        IsolatePlate(matImg,z);
       	
       	//findSquares(matImg, squares);
        
		//drawSquares(original, squares);
	}
}

void SearchForShadow(Mat src,int uBoundary)
{
	Mat bigImg(src);
	float reductionFactor = 0.3;
	Size smallSize(src.cols*reductionFactor,src.rows*reductionFactor);
	Mat smallerImg = Mat::zeros( smallSize, src.type());
	//make a copy of src to smallerImg reduced to 30% of the original size
	resize(src,smallerImg,smallerImg.size(),0,0,INTER_CUBIC);
	
	smallerImg = TransitionToShadow(smallerImg, uBoundary);

	smallerImg = ExludeFalseShadowPixels(smallerImg, smallSize);

	vector<Vec4i> lines = mergeLines(smallerImg);
	
	lines = excludeDuplicateShadows(lines);
	
	CreateROIOfShadow(lines, bigImg, reductionFactor);
	
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


