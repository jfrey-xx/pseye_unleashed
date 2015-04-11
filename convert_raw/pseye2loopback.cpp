#include <opencv2/opencv.hpp>
#include <iostream>
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
  int width = WIDTH;
  int height = HEIGHT;
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
     width = atoi(op_width);
   }
   char * op_height = getCmdOption(argv, argv + argc, "--height");
   if (op_height) {
     height = atoi(op_height);
   }
   char * op_video_in = getCmdOption(argv, argv + argc, "--video-in");
   if (op_video_in) {
     video_device_in.assign(op_video_in);
   }
   char * op_video_out = getCmdOption(argv, argv + argc, "--video-out");
   if (op_video_out) {
     video_device_out.assign(op_video_out);
   }

  cout << "Using" << video_device_in << " for PS Eye, at " << width << "x" << height << endl;
  cout << "Using" << video_device_out << " for loopback" << endl;


  char wKey = -1;

  /***** setting up loopback *****/
  cout << "Init V4L loopback" << endl;

  OutLoop loop = OutLoop(video_device_out, width, height);

  /***** setting up ps eye *****/
  PSEyeWB pseye = PSEyeWB(video_device_in, width, height);

  // ESC to escape
  while(wKey != 27)
    {
    
      Mat wb = pseye.getFrame();
      imshow("PS eye WB " + video_device_in, wb);

      loop.sendFrame(wb);

      wKey = waitKey(1);
	  
      // we got something with SPACE
      if (wKey == 32) {
	cout << "trigger" << endl;
      }
    }

  cout << "Program ended" << endl;

  return 0;

}
