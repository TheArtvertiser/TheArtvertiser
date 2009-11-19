#ifndef _MULTIGRAB_H
#define _MULTIGRAB_H

#include "calibmodel.h"

class MultiGrab {
public:

	CalibModel model;

	MultiGrab(const char *modelfile="model.bmp") : model(modelfile) {}

	int init(bool cacheTraining, char *modelfile, char *avi_bg_path, int WIDTH, int HEIGHT, int V4LDEVICE);
	void grabFrames();
	void allocLightCollector();

	struct Cam {
		CvCapture *cam;
		IplImage *frame, *gray, *f;
		int width,height;
		planar_object_recognizer detector;
		LightCollector *lc;

		void setCam(CvCapture *c, int WIDTH, int HEIGHT);
		bool detect();

		Cam(CvCapture *c=0, int WIDTH=0, int HEIGHT=0) 
		{
			f=0;
			width=0;
			height=0;
			cam=0;
			lc=0;
			if (c) setCam(c, WIDTH, HEIGHT);
			frame=f;
			gray=0;
		}
		~Cam();
	};

	std::vector<Cam *> cams;
	struct Cam *foo;
};

bool add_detected_homography(int n, planar_object_recognizer &detector, CamCalibration &calib);
bool add_detected_homography(int n, planar_object_recognizer &detector, CamAugmentation &a);
IplImage *myQueryFrame(CvCapture *capture);
IplImage *myRetrieveFrame(CvCapture *capture);

#endif
