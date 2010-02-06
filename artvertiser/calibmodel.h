#ifndef CALIBMODEL_H
#define CALIBMODEL_H

#include <cv.h>
#include <highgui.h>
#include <garfeild.h>

class CalibModel {
public:
	CvPoint corners[4];
	CvPoint artvert_corners[4];

	LightMap map;
	CamAugmentation augm;

	CalibModel();
	~CalibModel();

	void useModelFile( const char *modelfile );

	bool buildCached(int nbcam, CvCapture *capture, bool cache, planar_object_recognizer &detector, bool dont_try_to_train );

    int getImageWidth() { return image_width; }
    int getImageHeight() { return image_height; }
private:
	IplImage *image;

	int image_width, image_height;

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
