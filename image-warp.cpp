#include "cv.h"
#include "highgui.h"

#define WIDTH 640
#define HEIGHT 480

int main( int argc, char** argv )
{

	IplImage *src = cvLoadImage("1.png", 3); 
	IplImage *warped = cvCreateImage(cvSize(src->width, src->height), 8, 3);

	CvMat* mmat = cvCreateMat(3,3,CV_32FC1); 

	CvPoint2D32f *c1 = new CvPoint2D32f[4];
	CvPoint2D32f *c2 = new CvPoint2D32f[4];
	c1[0].x = 126;   c1[0].y = 130;
	c1[1].x = 508;   c1[1].y = 37;
	c1[2].x = 109;   c1[2].y = 370;
	c1[3].x = 516;   c1[3].y = 429;

	/*
	c1[0].x = 0;   c1[0].y = 0;
	c1[1].x = 640;   c1[1].y = 0;
	c1[2].x = 0;   c1[2].y = 480;
	c1[3].x = 640;   c1[3].y = 480;
	*/

	c2[0].x = 73;   c2[0].y = 54;
	c2[1].x = 501;   c2[1].y = 144;
	c2[2].x = 43;   c2[2].y = 457;
	c2[3].x = 495;   c2[3].y = 399; 

    mmat = cvGetPerspectiveTransform(c1, c2, mmat);
    cvWarpPerspective(src, warped, mmat); 

	IplImage *i1=cvCreateImage(cvSize(src->width,src->height),src->depth,1);
	IplImage *i2=cvCreateImage(cvSize(src->width,src->height),src->depth,1);
	IplImage *diff=cvCreateImage(cvSize(src->width,src->height),src->depth,1);
	IplImage *display=cvCreateImage(cvSize(src->width,src->height),src->depth,1);

	cvCvtColor(src, i1,CV_BGR2GRAY);
	cvCvtColor(warped, i2,CV_BGR2GRAY);

	cvAbsDiff(i1,i2,diff);
	cvThreshold(diff, display, 12, 255, CV_THRESH_BINARY);

    cvNamedWindow( "diff", CV_WINDOW_AUTOSIZE );
	cvShowImage("diff", display);
    cvWaitKey();

    return 0;
    // all the images will be released automatically
}




