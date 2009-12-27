#ifndef _MULTIGRAB_H
#define _MULTIGRAB_H

#include "calibmodel.h"

class MultiGrab {
public:

	CalibModel model;

	MultiGrab(const char *modelfile="model.bmp") : model(modelfile) {}

	int init(bool cacheTraining, char *modelfile, char *avi_bg_path, int capture_width, int capture_height, int v4l_device, int detect_width, int detect_height );
	void grabFrames();
	void allocLightCollector();

	struct Cam {
		CvCapture *cam;
		IplImage *frame, *gray, *gray_detect, *f;
		int width,height;
		int detect_width, detect_height;
		planar_object_recognizer detector;
		LightCollector *lc;

		void setCam(CvCapture *c, int capture_width, int capture_height, int detect_width, int detect_height );
		bool detect();

		Cam(CvCapture *c=0, int _width=0, int _height=0, int _detect_width=320, int _detect_height=240)
		{
			f=0;
			width=0;
			height=0;
			detect_width=_detect_width;
			detect_height=_detect_height;
			cam=0;
			lc=0;
			if (c) setCam(c, _width, _height, _detect_width, _detect_height );
			frame=f;
			gray=0;
			gray_detect = 0;
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
