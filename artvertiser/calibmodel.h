#ifndef CALIBMODEL_H
#define CALIBMODEL_H

#include <cv.h>
#include <highgui.h>
#include <garfeild.h>

class CalibModel {
public:
	IplImage *image;
	CvPoint corners[4];
	CvPoint artvert_corners[4];

	LightMap map;
	CamAugmentation augm;

	CalibModel(const char *modelfile = "model.bmp");
	~CalibModel();

	bool buildCached(int nbcam, CvCapture *capture, bool cache, planar_object_recognizer &detector);

private:
	const char *win;
	const char *modelfile;

	enum State { TAKE_SHOT, CORNERS, ARTVERT_CORNERS };
	State state;
	int grab;
	static void onMouseStatic(int event, int x, int y, int flags, void* param);
	void onMouse(int event, int x, int y, int flags);
	bool interactiveSetup(CvCapture *capture);
};


#endif
