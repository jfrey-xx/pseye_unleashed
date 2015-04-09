#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include "v4ldevice.h"
#include "pseye.hpp"

using namespace cv;


/* Convert a packed array of n elements with vw useful bits into array of
 * zero-padded 16bit elements.
 *
 * @param src The source packed array, of size (n * vw / 8) bytes
 * @param dest The destination unpacked array, of size (n * 2) bytes
 * @param vw The virtual width of elements, that is the number of useful bits for each of them
 * @param n The number of elements (in particular, of the destination array), NOT a length in bytes
 */
static inline void convert_packed_to_16bit(uint8_t *src, uint16_t *dest, int vw, int n)
{
	unsigned int mask = (1 << vw) - 1;
	uint32_t buffer = 0;
	int bitsIn = 0;
	while (n--) {
		while (bitsIn < vw) {
			buffer = (buffer << 8) | *(src++);
			bitsIn += 8;
		}
		bitsIn -= vw;
		*(dest++) = (buffer >> bitsIn) & mask;
	}
}

// argo from http://web.stanford.edu/~sujason/ColorBalancing/simplestcb.html
// implementation from http://web.stanford.edu/~sujason/ColorBalancing/simplestcb.html
/// perform the Simplest Color Balancing algorithm
void SimplestCB(Mat& in, Mat& out, float percent) {
    assert(in.channels() == 3);
    assert(percent > 0 && percent < 100);
 
    float half_percent = percent / 200.0f;
 
    vector<Mat> tmpsplit; split(in,tmpsplit);
    for(int i=0;i<3;i++) {
        //find the low and high precentile values (based on the input percentile)
        Mat flat; tmpsplit[i].reshape(1,1).copyTo(flat);
        cv::sort(flat,flat,CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
        int lowval = flat.at<uchar>(cvFloor(((float)flat.cols) * half_percent));
        int highval = flat.at<uchar>(cvCeil(((float)flat.cols) * (1.0 - half_percent)));
	std::cout << lowval << " " << highval << std::endl;
        
        //saturate below the low percentile and above the high percentile
        tmpsplit[i].setTo(lowval,tmpsplit[i] < lowval);
        tmpsplit[i].setTo(highval,tmpsplit[i] > highval);
        
        //scale the channel
        normalize(tmpsplit[i],tmpsplit[i],0,255,NORM_MINMAX);
    }
    merge(tmpsplit,out);
}

// version with 32F matrix
void SimplestCB32F(Mat& in, Mat& out, float percent) {
    assert(in.channels() == 3);
    assert(percent > 0 && percent < 100);
 
    float half_percent = percent / 200.0f;
 
    vector<Mat> tmpsplit; split(in,tmpsplit);
    for(int i=0;i<3;i++) {
        //find the low and high precentile values (based on the input percentile)
        Mat flat; tmpsplit[i].reshape(1,1).copyTo(flat);
        cv::sort(flat,flat,CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
        float lowval32 = flat.at<float>(((float)flat.cols) * half_percent);
        float highval32 = flat.at<float>(((float)flat.cols) * (1.0 - half_percent));
	std::cout << "float: " << lowval32 << " " << highval32 << std::endl;
        
        //saturate below the low percentile and above the high percentile
        tmpsplit[i].setTo(lowval32,tmpsplit[i] < lowval32);
        tmpsplit[i].setTo(highval32,tmpsplit[i] > highval32);
        
        //scale the channel
        normalize(tmpsplit[i],tmpsplit[i],0,1,NORM_MINMAX);
    }
    merge(tmpsplit,out);
}

/**
 * Convert a packed array of n elements with vw useful bits into array of
 * 8bit elements, dropping LSB.
 *
 * @param src The source packed array, of size (n * vw / 8) bytes
 * @param dest The destination unpacked array, of size (n * 2) bytes
 * @param vw The virtual width of elements, that is the number of useful bits for each of them
 * @param n The number of elements (in particular, of the destination array), NOT a length in bytes
 *
 * @pre vw is expected to be >= 8.
 */
static inline void convert_packed_to_8bit(uint8_t *src, uint8_t *dest, int vw, int n)
{
	uint32_t buffer = 0;
	int bitsIn = 0;
	while (n--) {
		while (bitsIn < vw) {
			buffer = (buffer << 8) | *(src++);
			bitsIn += 8;
		}
		bitsIn -= vw;
		*(dest++) = buffer >> (bitsIn + vw - 8);
	}
}

void print_mat(Mat mat) {
  //	  printf("%d ", mat.step);
  for (int j=0;j<mat.rows;j++) {
    printf("%d[",j);
      for (int i=0;i< mat.cols ;i++) {
  	printf("%d ", mat.data[mat.cols*j+i]);
  	}
  	    printf("]\n");
    }
// unsigned char *input = (unsigned char*)(mat.data);

//   for (int i=0;i<mat.rows;i++) {
//     printf("%d[",i);
//       for (int j=0;j< mat.cols ;j++) {
// 	printf("j%d[%d] ", i,input[mat.step*j+i]);
// 	}
// 	    printf("]\n");
//     }

}

int main ()
{

  CvSize ImageSize, ImageSizeR;
    unsigned char* ImageBuffer = NULL;
    int wKey = -1;


    ImageSize.width = 320;
    ImageSize.height = 240;

    ImageSizeR.width = 240;
    ImageSizeR.height = 320;

    //    cvNamedWindow( (char*)"Camera", 1 );

    printf("Program started\n");

    cv::Mat pack = Mat(ImageSize, CV_16UC1); // packed
    cv::Mat pack8 = Mat(ImageSize, CV_8UC1); // packed

    cv::Mat pOpenCVImage = Mat(ImageSize, CV_8UC4); //cvCreateImage(ImageSize , IPL_DEPTH_16U, 1 ); // Grayscale
    cv::Mat out = Mat(ImageSize, CV_8UC1);
    cv::Mat colorz = Mat(ImageSize, CV_8UC3);
    cv::Mat wb = Mat(ImageSize, CV_8UC3);
    cv::Mat colorz32 = Mat(ImageSize, CV_32FC3);
    cv::Mat wb32 = Mat(ImageSize, CV_32FC3);

    cv::Mat mat16 = Mat(ImageSize, CV_8UC2);  //cvCreateImage(ImageSize , IPL_DEPTH_16U, 3 ); // Color image

    cv::Mat mat16R = Mat(ImageSize, CV_8UC2);  //cvCreateImage(ImageSize , IPL_DEPTH_16U, 3 ); // Color image

    open_device((char*)"/dev/video0");
    init_device(ImageSize.width, ImageSize.height);


    printf("Start capturing\n");

    start_capturing();

    int rows = pOpenCVImage.rows;
    int cols = pOpenCVImage.cols;
    int num_el = rows*cols;
    int inputChan = 2;
    int len = num_el * inputChan;

// printf("Start capturing - %d by %d by %d for %d -\n",  pOpenCVImage.rows, pOpenCVImage.cols, pOpenCVImage.elemSize1(),     len);

  unsigned short int buffer[ImageSize.width*ImageSize.height];
  unsigned char buffer8[ImageSize.width*ImageSize.height];
    while(wKey == -1 )
    {
        ImageBuffer = snapFrame();
	//std::cout << "Length of array = " << sizeof(ImageBuffer) << ", bis: " <<sizeof(*ImageBuffer) << " ter: " << sizeof(*ImageBuffer) << std::endl;

        if( ImageBuffer != NULL )
        {
	  //memcpy( pOpenCVImage->imageData, ImageBuffer, pOpenCVImage->imageSize);
	    
	  memcpy( pOpenCVImage.data, ImageBuffer, len);
	  memcpy( mat16.data, ImageBuffer, len);
	  //      memcpy( mat16R.data, mat16.data, len);
	  //imshow( "all",  pOpenCVImage); 
	  //imshow( "mat16",  mat16); 

	  // vector<Mat> mat16chan(2);

	  //split(mat16, mat16chan);
	  // imshow("mat16 chan1", mat16chan[0]);
	  //imshow("mat16 chan2", mat16chan[1]);
	       
	  Mat out = PSEyeBayer2RGB(pOpenCVImage);
	  imshow("tadam", out);

	  //imshow( "chan1",  channels[0] ); 
	  //imshow( "chan2",  channels[1]); 
	  //imshow( "chan3",  channels[2] ); 
	  //imshow( "chan4",  channels[3]); 

	  //imshow( "Display window rgb",  pOpenCVImage ); 

	  //	       convert_packed_to_8bit(ImageBuffer, buffer8, 10, ImageSize.width * ImageSize.height );
	  //	       memcpy( pack8.data, buffer8, len/2);

	  //	       convert_packed_to_16bit(ImageBuffer, buffer, 10, ImageSize.width * ImageSize.height );
	  // memcpy( pack.data, buffer, len);


   printf("return\n");
	  colorz =out ;
	  imshow( "Display window rgb", colorz ); 

	  SimplestCB(colorz,wb,1);
	  imshow("AWB",wb);

   printf("return\n");

	  //	  Mat toto = cv::Mat(ImageSize, CV_32FC1, ImageSize) ; 
	  cv::normalize(colorz, colorz32, 0, 1, cv::NORM_MINMAX, CV_32FC3);

	  //	  colorz.convertTo(colorz32, CV_32FC1); 
	  //	  cvCvtScale(colorz,colorz32,1/255.,0);
	  imshow("tadam32", colorz32);
	  SimplestCB32F(colorz32,wb32,1);
	  imshow("AWB32",wb32);

	  //////////////////////////
	  // print_mat(pOpenCVImage);



	  ////////////////////////

	  // cvShowImage( (char*)"Camera2",MatRgb);
	  wKey = cvWaitKey(1);
        }
        else
	  {
            printf("No image buffer retrieved.\n");
            break;
	  }
    }

    cvDestroyWindow( (char*)"Camera" );
    stop_capturing();
    uninit_device();
    close_device();

    printf("Program ended\n");

    return 0;

}
