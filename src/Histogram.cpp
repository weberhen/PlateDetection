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

  for(uint i=0;i<todos.size();i+=jump)
  {
    //if(todos[i]!=0)
      v.push_back(todos[i]);
  }
  sort(v.begin(),v.end());
  float max = v[v.size()-1];
  float reductionFactor = (float)(max/(float)255);
  Mat finalHist(255,258,CV_32F);
  
  namedWindow("finalHist",WINDOW_AUTOSIZE);
  IplImage * finalIPL = new IplImage(finalHist);
  
  for(int i=0;i<258;i++)
  {cout<<"inserindo: "<<v[i]/reductionFactor<<endl;
    cvLine( finalIPL,
          cvPoint(max/reductionFactor,i),
          cvPoint(max/reductionFactor-v[i]/reductionFactor,i),
          Scalar(122,0,255), 1, CV_AA);
    //cvLine( finalIPL,
      // cvPoint(1,0),
        // cvPoint(1,v[v.size()-2]),
          //Scalar(122,0,255), 1, CV_AA);
    
  }

  //rotating image in 180o



  cvShowImage("finalHist",finalIPL);
  waitKey();
  cvSaveImage("soPlacaHist.png",finalIPL);
  //cvSaveImage("soPlacaHist.png",finalIPL);
  cout<<"hei: "<<v.size()<<endl;
  

return 0;
}