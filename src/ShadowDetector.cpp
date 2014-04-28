#include "ShadowDetector.hpp"
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265

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
	if(_structAsphaltInfo.median<100)
		_structAsphaltInfo.median=130;
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
				||((segmentSize>i*maxSegmentSizeRatio) || (segmentSize<i*minSegmentSizeRatio)))
					EraseLine(shadows, segmentSize, i, j);
				segmentSize=0;
			}
		}
		EraseLine(shadows, segmentSize, i, input.cols-2);
		segmentSize=0;
	}
	return shadows;
}

void IsolatePlate(Mat input, int x, int y,int z,int hplate, int wplate)
{
	Mat original = input.clone();
	
	input.convertTo(input, CV_32F);
	int sz[3] = {input.rows,input.cols,3};
    	Mat xMat;

	Sobel( input, xMat, CV_32F, 1, 0);

	Mat xi,xi2;
	cv::integral(xMat, xi, xi2, CV_64F);
	float mul1=0.7;
	float mul2=0.35;
	//for(int change=0;change<100;change++)
	{
		
		int winx_factor=wplate*mul1;
		int winy_factor=hplate*mul2;
	//	mul1=mul1+0.02;
	//	mul2=mul2+0.01;

		if(winy_factor<=0)
			winy_factor++;
		if(winx_factor<=0)
			winx_factor++;
		
		//cout<<"winx_factor: "<<winx_factor<<" mul1: "<<mul1<<" winy_factor: "<<winy_factor<<" mul2: "<<mul2<<endl;

		//int winy_size[3] = {winy1,winy2,winy3};
		//int winx_size[3] = {winx1,winx2,winx3};
		int winy_size[3] = {winy_factor-1,winy_factor,winy_factor+1};
		int winx_size[3] = {winy_factor-1,winy_factor,winy_factor+1};

		//cout<<x<<" "<<y<<endl;


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

		//CalcHistogram( so );
		//cout<<"cols: "<<so.cols<<" rows: "<<so.rows<<" so.rows*0.5: "<<so.rows*0.5<<" so.cols*0.5: "<<so.cols*0.5<<" so.rows*0.9: "<<so.rows*0.9<<" so.cols*0.9: "<<so.cols*0.9<<endl;
		float T = 0.5*(so.at<float>(so.rows*median,so.cols*median)+(so.at<float>(so.rows*percentil,so.cols*percentil)));
		
		for(int i=0;i<srows;i++){
			for(int j=0;j<scols;j++)
			{
				if(ss.at<float>(i,j)<T)
					input.at<float>(i,j)=0.;
				else
					input.at<float>(i,j)=255.;
			}
		}
		//namedWindow("step",	WINDOW_AUTOSIZE);
		if(!onRPI)
		imshow("BW",input);
		//waitKey();
		
	}
	//waitKey();
	//cout<<"cols: "<<input.cols<<" rows: "<<input.rows<<" z: "<<z<<endl;
	
	ConnectedComponents(input, original, original, x, y,z,hplate,wplate);
	    
}

void CreateROIOfShadow(vector<Vec4i> lines, Mat hole_img, float reductionFactor)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//CAMERA PROJECTION
	//%
	//%  Sistema de coordenadas do mundo: (altura, largura, profundidade) (x,y,z)
	//%  Sistema de coordenadas da imagem: (vertical, horizontal) (u,v)
	//%
	int leftx,righty,rightx,lefty;
	float hbase_min, hbase_max;
	for(unsigned int i =0;i<lines.size();i++) //iterates over the shadows
	{
		//recover the coordinates (u,v) from the shadow
		Vec4i l = lines[i]; 
		//convert the values to the actual size of the picture
		leftx=l[0]/reductionFactor;
		lefty=l[1]/reductionFactor;
		rightx=l[2]/reductionFactor;
		righty=l[3]/reductionFactor;
		
		//height of the camera relative to the ground(meters)
		float h=1.2;

		/*CAMERA TRANSFORMATIONS*/
		
		// Dado um ponto (u,v) na imagem no ground plane, projeta para (y,z,0) no
		// mundo. Dadas uma altura hbase, re-projeta uma placa com dimensoes wplate
		// x hplate na posicao (y,z,hbase)
		
		//principal axis
		float u0=hole_img.rows;
		float v0=hole_img.cols;
						
		//The angle that the camera does in relation to the y axis
		float beta_grades = -19.0;
		float beta = radians(beta_grades);
		
		//focal lenght
		float fu = 674.3;
		float fv = 674.3;

		//The angle that the camera does in relation to the z axis
		float alpha_grades = 0;
		float alpha = radians(alpha_grades);

		//plate height(meters)
		float hplate=0.13;
		
		//plate width(meters)
		float wplate=0.4;

		//minimum height where the plate will be searched(meters)
		float hbase_min_num = 0.3;

		//maximum height where the plate will be searched(meters) 
		float hbase_max_num = 1.2;

		float hbase= hbase_min_num;

		//coordinates of the shadow
		float u=righty;
		float v=(rightx+leftx)/2;


		//Matriz de rotação exata eixo x
		Mat Rx = (Mat_<float>(3,3)<<
			       1,          0, 0,
			0, cos(alpha),-sin(alpha),
			0, sin(alpha), cos(alpha)); 
		
		//Matriz de rotação exata eixo y
		Mat Ry = (Mat_<float>(3,3)<<
			cos(beta), 0, -sin(beta),
			        0, 1,          0,
			sin(beta), 0,  cos(beta));
		
		Mat Ra = (Mat_<float>(3,3));
		
		//Matriz de rotação total (aproximada)
		Ra = Ry*Rx;

		//translation "vector". It is the position of the camera in world coordinatess
		Mat X0 = (Mat_<float>( 3, 1) << h, 0, 0); 

		//extracted from MATLAB
		Mat XCl = (Mat_<float>( 3, 1)<<
   			(h*sin(beta)*(fu*cos(beta) - u0*sin(beta) + u*sin(beta)))/(u0*cos(beta) - u*cos(beta) + fu*sin(beta)) - cos(beta)*(h - hbase),
            (fu*h*v0 - fu*h*v)/(fv*u0*cos(beta) - fv*u*cos(beta) + fu*fv*sin(beta)) - wplate/2,
 			-sin(beta)*(h - hbase) - (h*cos(beta)*(fu*cos(beta) - u0*sin(beta) + u*sin(beta)))/(u0*cos(beta) - u*cos(beta) + fu*sin(beta)));

		float uu = fu*XCl.at<float>(0)/XCl.at<float>(2);
		float ubr = u0-uu;
		float vv = fu*XCl.at<float>(1)/XCl.at<float>(2);
		float vtl = vv+v0;

		//extracted from MATLAB
		Mat XCl2 = (Mat_<float>( 3, 1)<<
		cos(beta)*(hbase - h + hplate) + (h*sin(beta)*(fu*cos(beta) - u0*sin(beta) + u*sin(beta)))/(u0*cos(beta) - u*cos(beta) + fu*sin(beta)),
        wplate/2 + (fu*h*v0 - fu*h*v)/(fv*u0*cos(beta) - fv*u*cos(beta) + fu*fv*sin(beta)),
 		sin(beta)*(hbase - h + hplate) - (h*cos(beta)*(fu*cos(beta) - u0*sin(beta) + u*sin(beta)))/(u0*cos(beta) - u*cos(beta) + fu*sin(beta))
 		);

 		uu = fu*XCl2.at<float>(0)/XCl2.at<float>(2);
		float utl = u0-uu;
		vv = fu*XCl2.at<float>(1)/XCl2.at<float>(2);
		float vbr = vv+v0;


		//ROI height
		//min
		hbase= hbase_min_num;
		Mat XCl3 = (Mat_<float>( 3, 1)<<
   		(h*sin(beta)*(fu*cos(beta) - u0*sin(beta) + u*sin(beta)))/(u0*cos(beta) - u*cos(beta) + fu*sin(beta)) - cos(beta)*(h - hbase + (3*hplate)/2),
        (fu*h*v0 - fu*h*v)/(fv*u0*cos(beta) - fv*u*cos(beta) + fu*fv*sin(beta)),
 		- sin(beta)*(h - hbase + (3*hplate)/2) - (h*cos(beta)*(fu*cos(beta) - u0*sin(beta) + u*sin(beta)))/(u0*cos(beta) - u*cos(beta) + fu*sin(beta))
 		);
 
 		uu = fu*XCl3.at<float>(0)/XCl3.at<float>(2);
		hbase_min = u0-uu;
		
		//ROI height
		//max
		hbase= hbase_max_num;
		Mat XCl4 = (Mat_<float>( 3, 1)<<
   		(h*sin(beta)*(fu*cos(beta) - u0*sin(beta) + u*sin(beta)))/(u0*cos(beta) - u*cos(beta) + fu*sin(beta)) - cos(beta)*(h - hbase + (3*hplate)/2),
        (fu*h*v0 - fu*h*v)/(fv*u0*cos(beta) - fv*u*cos(beta) + fu*fv*sin(beta)),
 		- sin(beta)*(h - hbase + (3*hplate)/2) - (h*cos(beta)*(fu*cos(beta) - u0*sin(beta) + u*sin(beta)))/(u0*cos(beta) - u*cos(beta) + fu*sin(beta))
 		);
 
 		uu = fu*XCl4.at<float>(0)/XCl4.at<float>(2);
		hbase_max = u0-uu;
		if(hbase_max<0)
			hbase_max=0;

		Point Pt1,Pt2;
		Pt2.x=vtl;
		Pt2.y=utl;
		Pt1.x=vbr;
		Pt1.y=ubr;

		Point shadow2,shadow1;
		shadow1.x=leftx;
		shadow1.y=righty;
		shadow2.x=rightx;
		shadow2.y=righty;
		
		RNG rng(-1);
		Scalar color=(255,155,255);
		//rectangle( hole_img, Pt1, Pt2, color, rng.uniform(1,1), CV_AA );
		//line(hole_img,shadow1,shadow2,color,rng.uniform(1,1),CV_AA);

		int x = leftx;
		
		int width = (rightx-leftx);
		int height = hbase_min-hbase_max;
		int y = righty-height;
		if(y<0)
			y=0;
		cv::Rect myROI(x,y,width,height);

		int hplate_im=ubr-utl;
		int wplate_im=vbr-vtl;

		//cout<<x<<" "<<y<<" "<<width<<" "<<height<<" "<<hbase_min<<" "<<hbase_max<<endl;
		//cout<<hole_img.rows<<" "<<hole_img.cols<<endl;
		//waitKey();
		Mat matImg = hole_img(myROI);

		//imshow("step",matImg);

		//z is the distance between the camera and the car 
        //being evaluated through its position in the image
        int z = righty; //y + height;

	    IsolatePlate(matImg, leftx, righty,z, hplate_im, wplate_im);
	}
	
	//////////////////////////////////////////////////////////////////////
	
	
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


