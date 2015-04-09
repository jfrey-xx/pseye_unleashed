#include <opencv2/opencv.hpp>
#include <iostream>
#include "v4ldevice.h"
#include "pseye.hpp"
#include "outloop.hpp"
#include <string.h>

using namespace cv;
using namespace std;

#define WIDTH 640
#define HEIGHT 480
#define DEVICE "/dev/video1"
#define FPS 30

int main ()
{

  CvSize ImageSize;
  unsigned char* ImageBuffer = NULL;
  char wKey = -1;

  ImageSize.width = WIDTH;
  ImageSize.height = HEIGHT;
  string video_device_out = DEVICE;

  cout << "Program started" << endl;

  /***** setting up loopback *****/
  cout << "Init V4L loopback" << endl;

  OutLoop loop = OutLoop(video_device_out, ImageSize.width, ImageSize.height, FPS);

  /***** setting up ps eye *****/

  cv::Mat pOpenCVImage = Mat(ImageSize, CV_8UC4); //cvCreateImage(ImageSize , IPL_DEPTH_16U, 1 ); // Grayscale
  cv::Mat out = Mat(ImageSize, CV_8UC1);
  cv::Mat colorz32 = Mat(ImageSize, CV_32FC3);
  cv::Mat wb32 = Mat(ImageSize, CV_32FC3);

  open_device((char*)"/dev/video0");
  init_device(ImageSize.width, ImageSize.height);

  cout << "Start capturing" << endl;

  start_capturing();

  int rows = pOpenCVImage.rows;
  int cols = pOpenCVImage.cols;
  int num_el = rows*cols;
  int inputChan = 2;
  int len = num_el * inputChan;

  // ESC to escape
  while(wKey != 27)
    {
      ImageBuffer = snapFrame();

      if( ImageBuffer != NULL )
        {
	    
	  memcpy( pOpenCVImage.data, ImageBuffer, len);
	       
	  out = PSEyeBayer2RGB(pOpenCVImage);
	  imshow("tadam", out);

	  // convert to 32F and normalize
	  normalize(out, colorz32, 0, 1, cv::NORM_MINMAX, CV_32FC3);

	  wb32 = SimplestCB(colorz32,1);
	  imshow("AWB",wb32);

	  loop.sendFrame(wb32);

	  wKey = waitKey(1);
	  
	  // we got something with SPACE
	  if (wKey == 32) {
	    cout << "trigger" << endl;
	    }
        }
      else
	{
	  cout << "No image buffer retrieved." << endl;
	  break;
	}
    }

  cvDestroyWindow( (char*)"Camera" );
  stop_capturing();
  uninit_device();
  close_device();

  cout << "Program ended" << endl;

  return 0;

}
