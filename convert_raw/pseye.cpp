#include "pseye.hpp"
#include <iostream>
#include "opencv2/opencv.hpp"

using namespace cv;

// get a 8UC1 matrix fetched from a PS Eye with raw patch, returns a 8UC3 matrix
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

// auto white-balance
// algo from http://web.stanford.edu/ps~sujason/ColorBalancing/simplestcb.html
// implementation from http://web.stanford.edu/~sujason/ColorBalancing/simplestcb.html
/// perform the Simplest Color Balancing algorithm
// returns NULL if input matrix not supported
cv::Mat SimplestCB(Mat in, float percent) {
  assert(percent > 0 && percent < 100);

  // two sligthly different calls depending on the data type, default to float
  bool type_float = true;
  int max_value;

  switch (in.type()) {
  case CV_8UC3:
    max_value = 255; //TODO: trivial to add 16
    type_float = false;
    break;
  case CV_32FC3:
  case CV_64FC3:
    max_value = 1;
    type_float = true;
    break;
  default:
    assert(1); // should not be here
  }

  cv::Mat out = in.clone();

  float half_percent = percent / 200.0f;
 
  vector<Mat> tmpsplit; split(in,tmpsplit);
  for(int i=0;i<3;i++) {
    //find the low and high precentile values (based on the input percentile)
    Mat flat; tmpsplit[i].reshape(1,1).copyTo(flat);
    cv::sort(flat,flat,CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
    float lowval;
    float highval;

    if (type_float) {
      lowval = flat.at<float>(((float)flat.cols) * half_percent);
      highval = flat.at<float>(((float)flat.cols) * (1.0 - half_percent));   
    }
    else {
      lowval = flat.at<uchar>(cvFloor(((float)flat.cols) * half_percent));
      highval = flat.at<uchar>(cvCeil(((float)flat.cols) * (1.0 - half_percent)));   
    }

    //saturate below the low percentile and above the high percentile
    tmpsplit[i].setTo(lowval,tmpsplit[i] < lowval);
    tmpsplit[i].setTo(highval,tmpsplit[i] > highval); 
    
    //scale the channel
    if (type_float) {
      normalize(tmpsplit[i],tmpsplit[i],0,max_value,NORM_MINMAX);
      }
    else {
      normalize(tmpsplit[i],tmpsplit[i],0,max_value,NORM_MINMAX);
    }
  }
  merge(tmpsplit,out);
  return out;
}
