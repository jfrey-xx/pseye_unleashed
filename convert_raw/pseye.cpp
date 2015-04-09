#include "pseye.hpp"
#include <iostream>
#include "opencv2/opencv.hpp"

using namespace cv;

// get a matrice fetched from a PS Eye with raw patch, return a 8UCC3 matrix
Mat PSEyeBayer2RGB(Mat src) {
  int width = src.cols;
  int height = src.rows;
  Mat tmp = Mat(height, width, CV_8UC1);
  Mat out = Mat(height, width, CV_8UC3);
  Size area_size = Size(width/4,height/2);
  Point start_even = Point(0,0);
  Point start_odd = Point(width/2,0);

  // split img to get our 4 channels
  vector<Mat> channels(4);
  split(src, channels);

  // get ROIs out of noise
  // Mat GBg1
  Mat g1 = Mat(channels[0], Rect(start_even,area_size));
  // Mat GBb1
  Mat b1 =  Mat(channels[0], Rect(start_odd, area_size));
  // Mat RGr2
  Mat r2 = Mat(channels[1], Rect(start_even,area_size));
  // Mat RGg2
  Mat g2 =  Mat(channels[1], Rect(start_odd, area_size));
  // Mat GBg2
  Mat g3 = Mat(channels[2], Rect(start_even,area_size));
  // Mat GBb2 =
  Mat b3 = Mat(channels[2],  Rect(start_odd, area_size));
  // Mat RGr2 
  Mat r4 = Mat(channels[3], Rect(start_even,area_size));
  // Mat RGg2 =
  Mat g4 = Mat(channels[3],  Rect(start_odd, area_size));

  // create back from 8 rois the bayer pattern
  // G1B1 G3B3 G1R1 ...
  // R2G2 R4G4 R2G2 ...
  for (int j=0;j<g1.rows;j++) {
    for (int i=0;i< g1.cols ;i++) {
      int step = g1.step*j+i;
      tmp.data[tmp.step*(j*2)+i*4] = g1.data[step];
      tmp.data[tmp.step*(j*2)+i*4+1] = b1.data[step];
      tmp.data[tmp.step*(j*2)+i*4+2] = g3.data[step];
      tmp.data[tmp.step*(j*2)+i*4+3] = b3.data[step];

      tmp.data[tmp.step*(j*2+1)+i*4] = r2.data[step];
      tmp.data[tmp.step*(j*2+1)+i*4+1] = g2.data[step];
      tmp.data[tmp.step*(j*2+1)+i*4+2] = r4.data[step];
      tmp.data[tmp.step*(j*2+1)+i*4+3] = g4.data[step];
    }
  }

  cv::cvtColor(tmp, out, CV_BayerGB2RGB);
  return out;
}
