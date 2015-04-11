#include "pseye.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include "v4ldevice.cpp"

using namespace cv;
using namespace std;

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

  cvtColor(tmp, out, CV_BayerGB2RGB);
  return out;
}

// carefull, number of channels of Mat in must match min/max_colors array size
void SimplestCBUpdateBoundaries(Mat in, float percent, float *min_colors, float *max_colors) {
  assert(percent > 0 && percent < 100);

  // two sligthly different calls depending on the data type, default to float
  bool type_float = true;

  switch (in.depth()) {
  case CV_8U:
  case CV_16U:
    type_float = false;
    break;
  case CV_32F:
  case CV_64F:
    type_float = true;
    break;
  default:
    assert(0); // should not be here
  }

  float half_percent = percent / 200.0f;

  vector<Mat> tmpsplit; split(in,tmpsplit);

  for(int i=0;i<(int)tmpsplit.size();i++) {
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
    min_colors[i] = lowval;
    max_colors[i] = highval;
  }
}

// Apply SimpleCB with previously computed boundaries
Mat SimplestCBApply(Mat in, float min_colors[], float max_colors[]) {

  int max_value;

  switch (in.depth()) {
  case CV_8U:
    max_value = 255; //TODO: trivial to add 16
    break;
  case CV_32F:
  case CV_64F:
    max_value = 1;
    break;
  default:
    assert(0); // should not be here
  }

  Mat out = in.clone();

  vector<Mat> tmpsplit; split(in,tmpsplit);

  for(int i=0;i<(int)tmpsplit.size();i++) {
    //saturate below the low percentile and above the high percentile
    tmpsplit[i].setTo(min_colors[i],tmpsplit[i] < min_colors[i]);
    tmpsplit[i].setTo(max_colors[i],tmpsplit[i] >  max_colors[i]); 
    //scale the channel
    cv::normalize(tmpsplit[i],tmpsplit[i],0,max_value,cv::NORM_MINMAX);
  }

  merge(tmpsplit,out);
  return out;
}

// auto white-balance
// algo from http://web.stanford.edu/ps~sujason/ColorBalancing/simplestcb.html
// implementation from http://web.stanford.edu/~sujason/ColorBalancing/simplestcb.html
/// perform the Simplest Color Balancing algorithm
// returns NULL if input matrix not supported
Mat SimplestCB(Mat in, float percent) {
  int n = in.channels();
  float min_colors[n];
  float max_colors[n];
  SimplestCBUpdateBoundaries(in, percent, min_colors, max_colors);
  return SimplestCBApply(in, min_colors, max_colors);
}

PSEyeWB::PSEyeWB(std::string video_device, int width, int height, bool continuousWB) {
  // init WB state
  setContinuousWB(continuousWB);
  for (int i = 0; i < 3; i++) {
    min_colors[i]=0;
    max_colors[i]=1;
  }

  ImageSize.width = width;
  ImageSize.height = height;

  pOpenCVImage = Mat(ImageSize, CV_8UC4);
  out = Mat(ImageSize, CV_8UC1);
  colorz32f = Mat(ImageSize, CV_32FC3);
  wb32f = Mat(ImageSize, CV_32FC3);
  wb = Mat(ImageSize, CV_8UC3);

  open_device((char*)video_device.c_str());
  init_device(ImageSize.width, ImageSize.height);

  cout << "Start capturing" << endl;
  start_capturing();

  int rows = pOpenCVImage.rows;
  int cols = pOpenCVImage.cols;
  int num_el = rows*cols;
  int inputChan = 2; // the driver uses 16 bits
  len = num_el * inputChan;
}

Mat PSEyeWB::getFrame() {
  ImageBuffer = snapFrame();

  if( ImageBuffer != NULL )
    {
      // retrieve bytes array
      memcpy( pOpenCVImage.data, ImageBuffer, len);
	  
      out = PSEyeBayer2RGB(pOpenCVImage);

      // convert to 32F and normalize
      normalize(out, colorz32f, 0, 1, cv::NORM_MINMAX, CV_32FC3);
      
      if (updateWB_next || updateWB_continuous) {
	for (int i = 0; i < 3; i++) {
	}
	updateWB_next = false;
      }
      // correct colors, still in 32f, then convert back to 8u before sending to loopback
      wb32f = SimplestCB(colorz32f,1);	  
      normalize(wb32f, wb, 0, 255, cv::NORM_MINMAX, CV_8UC3);

      // check how bytes are stored in the matrix, clone it should set things straight
      if (!wb.isContinuous()) {
	wb = wb.clone();
      }
    }
  else
    {
      cout << "No image buffer retrieved." << endl;
    }
  return wb;
}

void PSEyeWB::setContinuousWB(bool flag) {
  updateWB_next = flag;
  updateWB_continuous = flag;
}

PSEyeWB::~PSEyeWB(void) {
  cout << "Closing V4L device..." << endl;
  stop_capturing();
  uninit_device();
  close_device();
}
