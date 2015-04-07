#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include "v4ldevice.h"

using namespace cv;

void ShowImage(char* Name, IplImage* Img, int Attribute )
{
    cvNamedWindow(Name, Attribute );
    cvShowImage( Name, Img );
    cvWaitKey(0);
    cvDestroyWindow( Name );
}


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


int main ()
{

    CvSize ImageSize;
    unsigned char* ImageBuffer = NULL;
    int wKey = -1;

    ImageSize.width = 320;
    ImageSize.height = 240;

    cvNamedWindow( (char*)"Camera", 1 );

    printf("Program started\n");

    cv::Mat pack = Mat(ImageSize, CV_16UC1); // packed

    cv::Mat pOpenCVImage = Mat(ImageSize, CV_16UC1); //cvCreateImage(ImageSize , IPL_DEPTH_16U, 1 ); // Grayscale

    cv::Mat pColorCVImage = Mat(ImageSize, CV_16UC3);  //cvCreateImage(ImageSize , IPL_DEPTH_16U, 3 ); // Color image

    open_device((char*)"/dev/video0");
    init_device(ImageSize.width, ImageSize.height);


    printf("Start capturing\n");

    start_capturing();


    int rows = pOpenCVImage.rows;
int cols = pOpenCVImage.cols;
int num_el = rows*cols;
int len = num_el*pOpenCVImage.elemSize1();

    printf("Start capturing -%d-\n", len);
int SizeY = 480;
int SizeX = 640;

 short unsigned int buffer[ImageSize.width*ImageSize.height];
    while(wKey == -1 )
    {
        ImageBuffer = snapFrame();

        if( ImageBuffer != NULL )
        {
            //memcpy( pOpenCVImage->imageData, ImageBuffer, pOpenCVImage->imageSize);
	    
	       memcpy( pOpenCVImage.data, ImageBuffer, len);

	       convert_packed_to_16bit(ImageBuffer, buffer, 10, ImageSize.width * ImageSize.height );
	       memcpy( pack.data, buffer, len);
	 imshow( "tadam", pack ); 

	       printf("pix1[%d]", pOpenCVImage.data[43]);
      printf("pix2[%d]\n", pack.data[43]);

	 imshow( "orig", pOpenCVImage ); 
	  cv::cvtColor(pOpenCVImage,  pColorCVImage, CV_BayerRG2RGB);
           // cvCvtColor(pOpenCVImage,pColorCVImage,CV_BayerBG2RGB); // Create a color image from the raw data

            //cvShowImage( (char*)"Camera",pColorCVImage);
	  imshow( "Display window", pColorCVImage ); 
	
	  //	    printf("avant1.\n");
	    //cv::Mat Mat16Bit(SizeY, SizeX, CV_16UC1, pOpenCVImage);
	    cv::Mat Mat16Bit = pOpenCVImage.clone();
	     //cv::Mat Mat8Bit(SizeY, SizeX, CV_16UC1, pOpenCVImage);
	    //	    printf("avant2.\n");
cv::Mat Mat8Bit = Mat16Bit.clone();
//printf("avant3.\n");
Mat8Bit.convertTo(Mat8Bit, CV_8UC3, 0.0625);
//printf("avant4.\n");
cv::Mat MatRgb(SizeY, SizeX, CV_8UC3);
//printf("avant5.\n");
cv::cvtColor(Mat8Bit, MatRgb, CV_BayerBG2RGB);
//printf("avant6.\n");

imshow( "Display window 8", Mat8Bit ); 
imshow( "Display window rgb", MatRgb ); 

	   // cvShowImage( (char*)"Camera2",MatRgb);
            wKey = cvWaitKey(10);
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
