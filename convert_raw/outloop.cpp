#include "outloop.hpp"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

using namespace std;

OutLoop::OutLoop(string video_device, int width, int height) {
  this->width = width;
  this->height = height;

  cout << "Openning " << video_device << " ..." << endl;
  int fd = open(video_device.c_str(), O_RDWR);
  assert(fd>=0);
  
  this->fd = fd;

  // what should handle
  struct v4l2_capability vid_caps;
  cout << "Checking video cpapabilities..." << endl;
  int ret_code = ioctl(fd, VIDIOC_QUERYCAP, &vid_caps);
  assert(ret_code != -1);

  std::cout << "Configure loopback..." << std::endl;
  // configure for our RBG device
  struct v4l2_format vid_format_rgb;
  vid_format_rgb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  vid_format_rgb.fmt.pix.width = width;
  vid_format_rgb.fmt.pix.height = height;
  vid_format_rgb.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
  vid_format_rgb.fmt.pix.sizeimage = width * height * 3;
  vid_format_rgb.fmt.pix.field = V4L2_FIELD_NONE;
  vid_format_rgb.fmt.pix.bytesperline = width * 3;
  vid_format_rgb.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;
  ret_code = ioctl(fd, VIDIOC_S_FMT, &vid_format_rgb);
  assert(ret_code != -1);
}

// expects to get 8UC3 matrix!
  int OutLoop::sendFrame(Mat frame) {
    assert(frame.type() == CV_8UC3);
    // TODO: assertion about matrix size
    return write(fd, frame.data, width * height * 3);
}
