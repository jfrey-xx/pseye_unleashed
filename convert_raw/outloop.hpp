#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class OutLoop {
public:
  OutLoop(string video_device, int width, int height);
  ~OutLoop(void);
  int sendFrame(Mat frame);
private:
  string video_device;
  int fd;
  int width, height;
};
