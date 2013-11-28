#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

int CalcHistogram( Mat src)
{
  
  int jump = (src.cols*src.rows)/255;
  if(jump<1)
    jump=1;
  Vector<int>v;
  Vector<int>todos; //vetor com todos os desvios padrao
  cout<<"src.size(): "<<src.cols*src.rows<<" jump: "<<jump<<endl;

  for(int i=0;i<src.rows;i++)
  {
    for(int j=0;j<src.cols;j++)
    {
      todos.push_back(src.at<float>(i,j)) ;
    }
  }

  for(int i=0;i<todos.size();i+=jump)
  {
    //if(todos[i]!=0)
      v.push_back(todos[i]);
  }
  sort(v.begin(),v.end());
  float max = v[v.size()-1];
  float reductionFactor = (float)(max/(float)255);
  Mat finalHist(max,255,CV_32F);
  
  namedWindow("finalHist",WINDOW_AUTOSIZE);
  IplImage * finalIPL = new IplImage(finalHist);
  
  for(int i=0;i<v.size();i++)
  {cout<<"inserindo: "<<v[i]<<endl;
    cvLine( finalIPL,
          cvPoint(i,0),
          cvPoint(i,v[i]),
          Scalar(122,0,255), 1, CV_AA);
    //cvLine( finalIPL,
      // cvPoint(1,0),
        // cvPoint(1,v[v.size()-2]),
          //Scalar(122,0,255), 1, CV_AA);
    
  }

  cvShowImage("finalHist",finalIPL);
  waitKey();
  cvSaveImage("adesivoHist.png",finalIPL);
  //cvSaveImage("soPlacaHist.png",finalIPL);
  cout<<"hei: "<<v.size()<<endl;
  

return 0;
}