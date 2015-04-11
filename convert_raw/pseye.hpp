#include <opencv2/opencv.hpp>
#include <string.h>

// get a 8UC1 matrix fetched from a PS Eye with raw patch, returns a 8UC3 matrix
cv::Mat PSEyeBayer2RGB(cv::Mat src);

/// perform the Simplest Color Balancing algorithm
cv::Mat SimplestCB(cv::Mat in, float percent);

// adjust white balance on trigger
class PSEyeWB {
public:
  PSEyeWB(std::string video_device, int width, int height);
  ~PSEyeWB(void);
  // returns 8UC3 matrix (will return last captured frame if a problem occurs)
  cv::Mat getFrame();
private:
  // hold clipping values for color channels
  int min_colors[3];
  int max_colors[3];
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
};
