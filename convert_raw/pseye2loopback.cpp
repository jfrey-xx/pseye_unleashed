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
#define DEVICE_IN "/dev/video0"
#define DEVICE_OUT "/dev/video1"

int main ()
{

  CvSize ImageSize;
  unsigned char* ImageBuffer = NULL;
  char wKey = -1;

  ImageSize.width = WIDTH;
  ImageSize.height = HEIGHT;
  string video_device_out = DEVICE_OUT;
  string video_device_in = DEVICE_IN;

  cout << "Program started" << endl;

  /***** setting up loopback *****/
  cout << "Init V4L loopback" << endl;

  OutLoop loop = OutLoop(video_device_out, ImageSize.width, ImageSize.height);

  /***** setting up ps eye *****/

  cv::Mat pOpenCVImage = Mat(ImageSize, CV_8UC4);
  cv::Mat out = Mat(ImageSize, CV_8UC1);
  cv::Mat colorz32f = Mat(ImageSize, CV_32FC3);
  cv::Mat wb32f = Mat(ImageSize, CV_32FC3);
  cv::Mat wb = Mat(ImageSize, CV_8UC3);

  open_device((char*)video_device_in.c_str());
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
	  // retrieve bytes array
	  memcpy( pOpenCVImage.data, ImageBuffer, len);
	  
	  out = PSEyeBayer2RGB(pOpenCVImage);
	  imshow("tadam", out);

	  // convert to 32F and normalize
	  normalize(out, colorz32f, 0, 1, cv::NORM_MINMAX, CV_32FC3);
	  
	  // correct colors, still in 32f, then convert back to 8u before sending to loopback
	  wb32f = SimplestCB(colorz32f,1);
	  imshow("AWB",wb32f);
	  
	  normalize(wb32f, wb, 0, 255, cv::NORM_MINMAX, CV_8UC3);
	  // check how bytes are stored in the matrix, clone it should set things straight
	  if (!wb.isContinuous()) {
	    wb = wb.clone();
	  }

	  loop.sendFrame(wb);

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
