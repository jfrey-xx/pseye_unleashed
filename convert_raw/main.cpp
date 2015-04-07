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



int main ()
{

    //IplImage* pOpenCVImage;
    //IplImage* pColorCVImage;
  
  //Mat pOpenCVImage;
  //Mat pColorCVImage;
    CvSize ImageSize;
    unsigned char* ImageBuffer = NULL;
    int wKey = -1;

    ImageSize.width = 640;
    ImageSize.height = 480;

    cvNamedWindow( (char*)"Camera", 1 );


    printf("Program started\n");


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

    printf("Start capturing%d", len);
int SizeY = 480;
int SizeX = 640;
    while(wKey == -1 )
    {
        ImageBuffer = snapFrame();

        if( ImageBuffer != NULL )
        {
            //memcpy( pOpenCVImage->imageData, ImageBuffer, pOpenCVImage->imageSize);
	    
	       memcpy( pOpenCVImage.data, ImageBuffer, len);

	       printf("pix1: %d", pOpenCVImage.data[43]);

	 imshow( "orig", pOpenCVImage ); 
	  cv::cvtColor(pOpenCVImage,  pColorCVImage, CV_BayerBG2RGB);
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
