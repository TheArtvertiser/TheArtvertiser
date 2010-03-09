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

	bool buildCached(int nbcam, CvCapture *capture, bool cache, planar_object_recognizer &detector,
                  bool run_on_binoculars = false);

    int getImageWidth() { return image_width; }
    int getImageHeight() { return image_height; }

    /// update the interactive training on binoculars
    void interactiveTrainBinocularsUpdate(IplImage* frame, bool button_red, bool button_green, bool button_blue );
    /// draw the interactive training on binucolurs
    void interactiveTrainBinocularsDraw();

    /// true if we're running interactive training on the binoculars
    bool isInteractiveTrainBinocularsRunning() { return interactive_train_running; }
	/// tell the interactive training to abort
	void abortInteractiveTrainBinoculars() { if( interactive_train_running ) interactive_train_should_stop = true; }

	/// true if learning is in progress
	bool isLearnInProgress() { return learn_running; }
	/// get the training progress message
	const char* getLearnProgressMessage() { return progress_string; }

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

	bool interactiveTrainBinoculars();
	IplImage* train_working_image, *train_shot;
	IplTexture train_texture;
	bool interactive_train_running;
	bool train_should_proceed;
	bool interactive_train_should_stop;
	bool learn_running;


	bool debounce_green, debounce_redblue;
	int x, y;
	int corner_index;
	CvFont train_font;

	static char progress_string[2048];
	static void learnProgressionFunc( int phase, int current, int total ); 
};


#endif
