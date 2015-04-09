#include "opencv2/opencv.hpp"
#include "pseye.hpp"

using namespace cv;

int main(int, char**)
{

  CvSize ImageSize;
  ImageSize.width = 640;
  ImageSize.height = 480;


  VideoCapture cap(0); // open the default camera
  if(!cap.isOpened())  // check if we succeeded
    return -1;
    
  //    toto();

  Mat edges;
  //namedWindow("edges",1);
  for(;;)
    {
      Mat frame= Mat(ImageSize, CV_8UC1);// =  Mat(ImageSize, CV_8UC4);
      //      cap.retrieve(frame); // get a new frame from camera
      cap >> frame ;

      imshow("frame", frame);
      if(waitKey(1) >= 0) break;

      //    Mat frame2 = Mat(ImageSize, CV_8UC4);
      //    frame2.data = frame.data;

	 
      // Mat dst=  Mat(ImageSize, CV_8UC4);
      //frame.convertTo(dst,CV_8UC4);

      //dst.data = frame.data;


      //Mat color = PSEyeBayer2RGB(frame);
      // Mat color = PSEyeBayer2RGB(dst);

      // imshow("color", color);

    }
  // the camera will be deinitialized automatically in VideoCapture destructor
  return 0;
}
