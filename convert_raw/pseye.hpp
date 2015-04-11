#include <opencv2/opencv.hpp>
#include <string.h>

// get a 8UC1 matrix fetched from a PS Eye with raw patch, returns a 8UC3 matrix
cv::Mat PSEyeBayer2RGB(cv::Mat src);

/// perform the Simplest Color Balancing algorithm
cv::Mat SimplestCB(cv::Mat in, float percent);

// adjust white balance on trigger
class PSEyeWB {
public:
  // continuous WB: will update WB at each frame
  PSEyeWB(std::string video_device, int width, int height, bool continuousWB = true);
  ~PSEyeWB(void);
  // returns 8UC3 matrix (will return last captured frame if a problem occurs)
  cv::Mat getFrame();
  // set flag to update WB on next frame -- for continousWB == false
  void updateWB();
  // enable or disable continous WB
  void setContinuousWB(bool flag);
private:
  // hold clipping values for color channels
  float min_colors[3];
  float max_colors[3];
  // holders for raw data and our different conversions
  CvSize ImageSize;
  unsigned char* ImageBuffer;
  cv::Mat pOpenCVImage;
  cv::Mat out;
  cv::Mat colorz32f;
  cv::Mat wb32f;
  cv::Mat wb;
  // len of the array byte retrieve from V4L
  int len;
  // interfal state for one-shot or continuous WB
  bool updateWB_next;
  bool updateWB_continuous;
};
