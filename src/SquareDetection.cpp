// The "Square Detector" program.
// It loads several images subsequentally and tries to find squares in
// each image

#include "SquareDetection.hpp"

int thresh = 50, N = 11;
const char* wndname = "Square Detection Demo";

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
double angle( Point pt1, Point pt2, Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}




// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
int findSquares( const Mat& image, vector<vector<Point> >& squares )
{
    squares.clear();
    
    Mat pyr, timg, gray0(image.size(), CV_8U), gray;
    
    // down-scale and upscale the image to filter out the noise
    pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
    pyrUp(pyr, timg, image.size());
    vector<vector<Point> > contours;
    
    // find squares in every color plane of the image
    for( int c = 0; c < 1; c++ )
    {
        int ch[] = {c, 0};
        mixChannels(&timg, 1, &gray0, 1, ch, 1);
        
        // try several threshold levels
        for( int l = 0; l < N; l++ )
        {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading
            if( l == 0 )
            {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging)
                Canny(gray0, gray, 100, 100,5);//thresh, 5);
                // dilate canny output to remove potential
                // holes between edge segments
                //dilate(gray, gray, Mat(), Point(-1,-1));
                //namedWindow("canny",CV_WINDOW_AUTOSIZE);
                //imshow("canny",gray);
            }
            else
            {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                gray = gray0 >= (l+1)*255/N;
            }
            // find contours and store them all as a list
            findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

            vector<Point> approx;
            
            // test each contour
            for( size_t i = 0; i < contours.size(); i++ )
            {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
                
                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation
                if( approx.size() == 4 &&
                    //fabs(contourArea(Mat(approx))) > 1000 &&
                    isContourConvex(Mat(approx)) )
                {
                    double maxCosine = 0;

                    for( int j = 2; j < 5; j++ )
                    {
                        // find the maximum cosine of the angle between joint edges
                        double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                        maxCosine = MAX(maxCosine, cosine);
                    }

                    // if cosines of all angles are small
                    // (all angles are ~90 degree) then write quandrange
                    // vertices to resultant sequence
                    if( maxCosine < 0.3 )
                        squares.push_back(approx);
                }
            }
        }
    }
}


// the function draws all the squares in the image
void drawSquares( Mat& img, const vector<vector<Point> >& squares )
{
    int width = 0, height = 0;
    int max_x=0, min_x=99999,max_y=0,min_y=99999;
    for( size_t i = 0; i < squares.size(); i++ )
    {
        const Point* pt = &squares[i][0];
        int n = (int)squares[i].size();
        //polylines(image, &p, &n, 1, true, Scalar(0,255,0), 3, CV_AA);

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
    }
    width = max_x-min_x;
    height = max_y-min_y;
    if((height < width/3) && (height < img.rows/2) && (width < img.cols/2) && (height < width/2) && (min_x<img.cols))
    {
       //printf("passed %d %d %d %d\n", min_x,min_y,max_x-min_x,max_y-min_y);
        
        cv::Rect myROI(min_x,min_y,max_x-min_x,max_y-min_y);
        
        Mat matImg = img(myROI);
        namedWindow("WOW",CV_WINDOW_AUTOSIZE);
        Size biggerSize(matImg.cols*2,matImg.rows*2);
        Mat biggerImg = Mat::zeros( biggerSize, matImg.type());
        resize(matImg, matImg, biggerImg.size(),0,0,INTER_CUBIC);
        imshow("WOW",matImg);
        // cropped image
        
    }
    //imshow(wndname, image);
}


/*int main(int argc, char** argv)
{
    static const char* names[] = { "pic1.png", "pic2.png", "pic3.png",
        "pic4.png", "pic5.png", "pic6.png", 0 };

    namedWindow( wndname, 1 );
    vector<vector<Point> > squares;
    
    for( int i = 0; names[i] != 0; i++ )
    {
        Mat image = imread(names[i], 1);
        if( image.empty() )
        {
            cout << "Couldn't load " << names[i] << endl;
            continue;
        }
        
        findSquares(image, squares);
        drawSquares(image, squares);

        int c = waitKey();
        if( (char)c == 27 )
            break;
    }

    return 0;
}*/
