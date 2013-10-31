#include "ShadowDetector.hpp"

Mat src_gray;
Mat dst, detected_edges;

int edgeThreshold = 1;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
String window_name = "Edge Map";

extern double _rho;
extern double _theta;
extern int _threshold;
extern double _minLineLenght;
extern double _maxLineGap;


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
	cout<<"car detected"<<endl;
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
		else //this will prevent the next for to execute
			return lines;
		for(size_t j=i+1;j<lines.size();j++)
		{
			Vec4i list = lines[j];
			//if j of element is similar to j of another (10% threshold)	
			if((element[0]>list[0]*0.9)&&(element[0]<list[0]*1.1))
				if((element[1]>list[1]*0.9)&&(element[1]<list[1]*1.1))
				{	
					//cout<<element[0]<<" é maior que "<<list[0]*0.8<<" e menor do que "<<list[0]*1.2<<endl;
					//cout<<"that is similarity"<<endl;
					//cout<<"new unique shadow is "<<min(element[0],list[0])<<" "<<element[1]<<" "<<max(element[2],list[2])<<" "<<element[1]<<endl;
					lines[i][0]=min(element[0],list[0]);
					lines[i][2]=max(element[2],list[2]);
					lines[i][3]=element[1];
					//cout<<"merging: "<<element[0]<<" "<<element[1]<<" "<<element[2]<<" "<<element[3]<<" "<<endl;
					//cout<<"with: "<<list[0]<<" "<<list[1]<<" "<<list[2]<<" "<<list[3]<<" "<<endl;

					lines.erase(lines.begin()+j);
					for(size_t i =0; i < lines.size();i++)
					{
						Vec4i l = lines[i];
						//cout<<"after merging: "<<l[0]<<" "<<l[1]<<" "<<l[2]<<" "<<l[3]<<" "<<endl;
					}
				}
		}
		
	}
	//cout<<"ANOTHER ROUND"<<endl;
	for(size_t i =0; i < lines.size();i++)
	{	
		Vec4i element = lines[i];
		for(size_t j=i+1;j<lines.size();j++)
		{
			Vec4i list = lines[j];
			//if j of element is similar to j of another (10% threshold)	
			if((element[1]>list[1]*0.8)&&(element[1]<list[1]*1.2)){
				//cout<<"element[0] "<<element[0]<<" element[2] "<<element[2]<<endl;
				//cout<<"list[0] "<<list[0]<<" list[2] "<<list[2]<<endl;
				if(((element[0]>list[0])&&(element[0]<list[2]))||
				   ((element[2]>list[0])&&(element[2]<list[2]))||
				   ((list[0]>element[0])&&(list[0]<element[2]))||
				   ((list[2]>element[0])&&(list[2]<element[2])))
				{	
					//cout<<element[1]<<" é maior que "<<list[1]*0.8<<" e menor do que "<<list[1]*1.2<<endl;
					//cout<<"that is similarity"<<endl;
					//cout<<"new unique shadow is "<<min(element[0],list[0])<<" "<<element[1]<<" "<<max(element[2],list[2])<<" "<<element[1]<<endl;
					j--;
					lines[i][0]=min(element[0],list[0]);
					lines[i][2]=max(element[2],list[2]);
					lines[i][3]=element[1];
					//cout<<"merging: "<<element[0]<<" "<<element[1]<<" "<<element[2]<<" "<<element[3]<<" "<<endl;
					//cout<<"with: "<<list[0]<<" "<<list[1]<<" "<<list[2]<<" "<<list[3]<<" "<<endl;

					lines.erase(lines.begin()+j+1);
					for(size_t in =0; in < lines.size();in++)
					{
						Vec4i l = lines[in];
						//cout<<"after merging: "<<l[0]<<" "<<l[1]<<" "<<l[2]<<" "<<l[3]<<" "<<endl;
					}
				}
			//cout<<"I agora vale "<<i<<" e J vale "<<j<<endl;
			}
		}
		
	}
	return lines;
}

void SearchForShadow(Mat src,int uBoundary)
{
	namedWindow("small", CV_WINDOW_AUTOSIZE);
	Size smallSize(src.cols*0.3,src.rows*0.3);
	int segmentSize=0;
	Mat smallerImg = Mat::zeros( smallSize, src.type());
	Mat dst = Mat::zeros(smallSize,src.type());
	Mat shadows = Mat::zeros(smallSize,src.type());
	resize(src,smallerImg,smallerImg.size(),0,0,INTER_CUBIC);
	resize(src, src, smallerImg.size(),0,0,INTER_CUBIC);

	vector<vector<Point> > contours;
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
				if(segmentSize<(0.10*smallerImg.cols)||
				  ((segmentSize>i*1.35) || (segmentSize<i*0.65)))
					EraseLine(shadows, segmentSize, i, j);
				//else
					//src = SurroundCar(src,i,j,segmentSize);
				segmentSize=0;
			}
		}
		EraseLine(shadows, segmentSize, i, smallerImg.cols-2);
		segmentSize=0;
	}
	vector<Vec4i> lines;
	//cout<<"rho "<<_rho<<" theta "<<_theta<<" threshold "<<_threshold<<" minLineLenght "<<_minLineLenght<<" maxLineGap "<<_maxLineGap<<endl;
	HoughLinesP(shadows,lines,_rho,_theta, _threshold,_minLineLenght,_maxLineGap);
	imshow("small", shadows);
	waitKey();
	cout<<"found "<<lines.size()<<" shadows"<<endl;
	/*for(size_t i =0; i < lines.size();i++)
	{
		Vec4i l = lines[i];
		cout<<"valor: "<<l[0]<<" "<<l[1]<<" "<<l[2]<<" "<<l[3]<<" "<<endl;
	}*/	
	lines = excludeDuplicateShadows(lines);
	for(int i =0;i<lines.size();i++)
	{
		Vec4i l = lines[i];
		src = SurroundCar(src,l[3],l[2],l[2]-l[0]);
	}	
	imshow("small", src);
	waitKey();
	/*cout<<endl<<endl;
	for(size_t i =0; i < lines.size();i++)
	{
		Vec4i l = lines[i];
		cout<<"FINAL: "<<l[0]<<" "<<l[1]<<" "<<l[2]<<" "<<l[3]<<" "<<endl;
	}*/
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


