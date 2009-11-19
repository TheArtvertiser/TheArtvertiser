
/*
 * Example application to capture from a webcam. 
 * Install OpenCV and HighGui development packages and compile with something
 * like:
 *
 * 	g++ -lcv -lcvaux -lhighgui -I/usr/local/include/opencv -o captureFromCam captureFromCam.cpp
 *
 * USAGE: ./captureFromCam -s <width> <height>
 * 
 * Julian Oliver, 2009: http://julianoliver.com
 */

#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <iostream>
#include <time.h>


int WIDTH = 320;
int HEIGHT = 240;
int frames = 0;
int c;

int main(int argc, char **argv)
{

	if (argc <=3)
	{
		std::cerr << "USAGE: ./captureFromCam -s <width> <height>\n\
        Try sizes below 1024x768.\n";
		exit(1);
	}
	for (int i=1; i< argc; i++) 
	{
		if (strcmp (argv[i], "-s") == 0)
		{
			if (WIDTH <=1024 && HEIGHT<=768)
			{
				WIDTH=atoi(argv[i+1]);
				HEIGHT=atoi(argv[i+2]);
			}
			else 
			{
				std::cout << 
				"Sizes too large.. \
				\nDefaulting to 320x240" 
				<< std::endl;
			}
		}
	}

	CvCapture *capture = 0;
	capture = cvCaptureFromCAM(1);

	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, WIDTH);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

    if( !capture )
    {
		std::cout << "could not capture from camera\n" << std::endl;
        return -1;
    }  
	cvNamedWindow("webcam capture", CV_WINDOW_AUTOSIZE);
	for (;;)
	{
		//double t = (double)cvGetTickCount();
		IplImage *frame = cvQueryFrame(capture);
		if (!frame)	
			break;
		cvShowImage("webcam capture", frame);
		//double t1 = (double)cvGetTickCount() -t;
		c = cvWaitKey(10);
		if( (char)c == 27 )
			break;
	}
	cvReleaseCapture(0);
	cvDestroyWindow("webcam capture");
	return 0;
}			
