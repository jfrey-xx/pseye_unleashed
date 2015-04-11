#include <opencv2/opencv.hpp>
#include <iostream>
#include "v4ldevice.h"
#include "pseye.hpp"
#include "outloop.hpp"
#include <string.h>
#include <algorithm>

// from http://stackoverflow.com/a/868894
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

using namespace cv;
using namespace std;

#define WIDTH 640
#define HEIGHT 480
#define DEVICE_IN "/dev/video0"
#define DEVICE_OUT "/dev/video1"

int main (int argc, char * argv[])
{
  CvSize ImageSize;
  ImageSize.width = WIDTH;
  ImageSize.height = HEIGHT;
  string video_device_out = DEVICE_OUT;
  string video_device_in = DEVICE_IN;

  /***** retrieving options ****/
  if(cmdOptionExists(argv, argv+argc, "--help"))
    {
      cout << "Usage:" << endl;
      cout << "\t --width 640 --height 480 \t set resolution" << endl;
      cout << "\t --video-in /dev/video0 \t to set PS Eye input device" << endl;
      cout << "\t --video-out /dev/video1 \t to set loopback output device" << endl;
      return 0;
    }

   char * op_width = getCmdOption(argv, argv + argc, "--width");
   if (op_width) {
     ImageSize.width = atoi(op_width);
   }
   char * op_height = getCmdOption(argv, argv + argc, "--height");
   if (op_height) {
     ImageSize.height = atoi(op_height);
   }
   char * op_video_in = getCmdOption(argv, argv + argc, "--video-in");
   if (op_video_in) {
     video_device_in.assign(op_video_in);
   }
   char * op_video_out = getCmdOption(argv, argv + argc, "--video-out");
   if (op_video_out) {
     video_device_out.assign(op_video_out);
   }

  cout << "Using" << video_device_in << " for PS Eye, at " << ImageSize.width << "x" << ImageSize.height << endl;
  cout << "Using" << video_device_out << " for loopback" << endl;

  unsigned char* ImageBuffer = NULL;
  char wKey = -1;

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
