#include <opencv2/opencv.hpp>

// get a 8UC1 matrix fetched from a PS Eye with raw patch, returns a 8UC3 matrix
cv::Mat PSEyeBayer2RGB(cv::Mat src);

/// perform the Simplest Color Balancing algorithm
cv::Mat SimplestCB(cv::Mat in, float percent);
