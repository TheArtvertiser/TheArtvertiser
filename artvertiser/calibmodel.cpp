#include "calibmodel.h"
#include "multigrab.h"

CalibModel::CalibModel(const char *modelfile)
	: modelfile (modelfile)
{
	image=0;
	win = "The Artvertiser 0.4";
}

CalibModel::~CalibModel()
{
	if (image) cvReleaseImage(&image);
}

CalibModel *objectPtr=0;

void CalibModel::onMouse(int event, int x, int y, int flags)
{
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		// try to grab something
		grab = -1;
		for (int i=0; i<4; i++)
		{
			int dx = x-corners[i].x;
			int dy = y-corners[i].y;
			if (sqrt((double)(dx*dx+dy*dy)) <10) {
				grab = i;
				break;
			}
		}
	}

	if (grab!=-1)
	{
		corners[grab].x = x;
		corners[grab].y = y;
	}

	if (event == CV_EVENT_LBUTTONUP)
	{
		grab=-1;
	}
}

bool CalibModel::buildCached(int nbcam, CvCapture *capture, bool cache, planar_object_recognizer &detector)
{

	detector.ransac_dist_threshold = 6;
	detector.max_ransac_iterations = 800;
	detector.ransac_stop_support = 50;
	detector.non_linear_refine_threshold = 15.0f;
	//detector.point_detector_tau = 10;

	// A lower threshold will allow detection in harder conditions, but
	// might lead to false positives.
	detector.match_score_threshold=.013f;

	//detector.min_view_rate=.1;
	//detector.views_number = 100;
	// damian below
    detector.min_view_rate = .2;

    static const int MAX_MODEL_KEYPOINTS = 100;     // maximum number of keypoints on the model
    static const int PATCH_SIZE = 16;               // patch size in pixels
    static const int YAPE_RADIUS = 3;               // yape radius
    static const int NUM_TREES = 25;                // num classifier trees
    static const int NUM_GAUSSIAN_LEVELS = 5;       // num gaussian levels

	// Should we train or load the classifier ?
	if(cache && detector.build_with_cache(
				string(modelfile), // mode image file name
				/*500,               // maximum number of keypoints on the model
				//200,               // maximum number of keypoints on the model
				32,                // patch size in pixels
				5,                 // yape radius. Use 3,5 or 7.
				//3,                 // yape radius. Use 3,5 or 7.
				12,                // number of trees for the classifier. Somewhere between 12-50
				//20,                // number of trees for the classifier. Somewhere between 12-50
				3*/                  // number of levels in the gaussian pyramid
				// damian below
				MAX_MODEL_KEYPOINTS, // max keypoints
				PATCH_SIZE, // patch size
				YAPE_RADIUS,                 // yape radius. Use 3,5 or 7.
				NUM_TREES,                // number of trees for the classifier. Somewhere between 12-50
				NUM_GAUSSIAN_LEVELS                  // number of levels in the gaussian pyramid

				))
	{
		// loading worked. Remember the region of interest.
		corners[0].x = detector.new_images_generator.u_corner1;
		corners[0].y = detector.new_images_generator.v_corner1;
		corners[1].x = detector.new_images_generator.u_corner2;
		corners[1].y = detector.new_images_generator.v_corner2;
		corners[2].x = detector.new_images_generator.u_corner3;
		corners[2].y = detector.new_images_generator.v_corner3;
		corners[3].x = detector.new_images_generator.u_corner4;
		corners[3].y = detector.new_images_generator.v_corner4;

        #ifdef USE_MULTITHREADCAPTURE
        IplImage* init_image = NULL;
        MultiThreadCapture* mtc = MultiThreadCaptureManager::getInstance()->getCaptureForCam(capture);
        int timeout = 10000;
        bool got = false;
        do {
            got = mtc->getCopyOfLastFrame( &init_image );
        }
        while ( !got &&
               !usleep( 100000 ) &&
               (timeout-=100) > 0 );
        if ( init_image == NULL )
        {
            printf("capture failed\n");
            return false;
        }
		image = cvLoadImage(modelfile, init_image->nChannels == 3);
		cvReleaseImage(&init_image);
		#else
		image = cvLoadImage(modelfile, cvQueryFrame(capture)->nChannels==3 );
		#endif
	}
	else
	{
		// ask the user the take a shot of the model
		if (!interactiveSetup(capture)) return false;

		// train the classifier to detect this model
		//if (!detector.build(image, 500, 32, 3, 12, 3,0, 0))
		//if (!detector.build(image, 200, 32, 5, 20, 3,0, 0))
		int working_roi[8] = { corners[0].x, corners[0].y,
            corners[1].x, corners[1].y,
            corners[2].x, corners[2].y,
            corners[3].x, corners[3].y
            };
        LEARNPROGRESSION progress;
		if (!detector.build(image,
                MAX_MODEL_KEYPOINTS, // max keypoints
				PATCH_SIZE, // patch size
				YAPE_RADIUS,                 // yape radius. Use 3,5 or 7.
				NUM_TREES,                // number of trees for the classifier. Somewhere between 12-50
				NUM_GAUSSIAN_LEVELS,      // number of levels in the gaussian pyramid
				progress,
				working_roi
				))
			return false;

		// save the image
		if (!cvSaveImage(modelfile, image)) return false;

		// and the region of interest (ROI)
		string roifn = string(modelfile) + ".roi";
		ofstream roif(roifn.c_str());
		if (!roif.good()) return false;
		for (int i=0;i<4; i++)
			roif << corners[i].x << " " << corners[i].y << "\n";
		roif.close();

		// and the trained classifier
		detector.save(string(modelfile)+".classifier");
	}

	float cn[4][2];
	for (int i=0; i<4; i++)
	{
		cn[i][0] = corners[i].x;
		cn[i][1] = corners[i].y;
	}
	cout << corners[1].x << " " << corners[1].y << "\n" << endl;

	// prepare the light calibration reference
	return map.init(nbcam, image, cn, 8, 6);
}

static void putText(IplImage *im, const char *text, CvPoint p, CvFont *f1)
{
	cvPutText(im,text,p,f1, cvScalar(0,255, 255));
}


IplImage *myRetrieveFrame(CvCapture *capture)
{
    #ifdef USE_MULTITHREADCAPTURE
    assert(false && "don't call myRetrieveFrame when USE_MULTITHREADCAPTURE");
    #endif

	static IplImage *s=0;
	IplImage *frame =cvRetrieveFrame(capture);
	if (frame == 0) return 0;
	IplImage *ret=frame;
	if (frame->nChannels==1) {
	    printf("  PERFORMANCE WARNING: myRetrieveFrame converting colour\n");
		if (!s) s=cvCreateImage(cvSize(frame->width,frame->height), IPL_DEPTH_8U, 3);
		cvCvtColor(frame,s,CV_GRAY2BGR);
		ret = s;
	}
	if (ret->origin) {
	    printf("  PERFORMANCE WARNING: myRetrieveFrame flipping\n");
		if (!s) s=cvCreateImage(cvSize(frame->width,frame->height), IPL_DEPTH_8U, 3);
		cvFlip(ret, s);
		ret->origin=0;
		ret = s;
	}
	return ret;
}

IplImage *myQueryFrame(CvCapture *capture)
{
    #ifdef USE_MULTITHREADCAPTURE
    assert(false && "don't call myQueryFrame when USE_MULTITHREADCAPTURE");
    #endif

	cvGrabFrame(capture);
	return myRetrieveFrame(capture);
}



bool CalibModel::interactiveSetup(CvCapture *capture)
{

	CvFont font, fontbold;

	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, .5, .5, 0, 0, CV_AA);

	cvNamedWindow(win, CV_WINDOW_AUTOSIZE);
	grab=-1;

	objectPtr = this;
	cvSetMouseCallback(win, onMouseStatic, this);

	bool pause=false;
	#ifdef USE_MULTITHREADCAPTURE
    IplImage *frame_gray, *frame = NULL;
    FTime* timestamp = NULL;
    MultiThreadCapture* mtc = MultiThreadCaptureManager::getInstance()->getCaptureForCam(capture);
    int timeout = 10000;
    bool got = false;
    do {
        got = mtc->getLastProcessedFrame( &frame_gray, &frame, &timestamp );
    }
    while ( !got &&
           !usleep( 100000 ) &&
           (timeout-=100) > 0 );
    if ( frame == NULL )
    {
        printf("capture failed\n");
        return false;
    }
    #else
	IplImage *frame = myQueryFrame(capture);
	#endif
	IplImage *shot=0, *text=0;

	state = TAKE_SHOT;

	bool accepted =false;
	while (!accepted) {

		// wait for a key
		char k = cvWaitKey(10);

		if (k==27 || k=='q') {
			if (shot) cvReleaseImage(&shot);
			if (text) cvReleaseImage(&text);
			return false;
		}

		// clear text or grab the image to display
		if (!pause || shot==0) {
            #ifdef USE_MULTITHREADCAPTURE
            bool got = mtc->getLastProcessedFrame( &frame_gray, &frame, NULL,/*block until available*/true );
            if ( !got )
                continue;
            #else
			frame = myQueryFrame(capture);
			#endif
			if (!text) {
				text=cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
				int d = 30;
				corners[0].x = d;
				corners[0].y = d;
				corners[1].x = frame->width-d;
				corners[1].y = d;
				corners[2].x = frame->width-d;
				corners[2].y = frame->height-d;
				corners[3].x = d;
				corners[3].y = frame->height-d;
			}
			if (frame->nChannels==1)
				cvCvtColor(frame, text, CV_GRAY2BGR);
			else
				cvCopy(frame,text);
		} else {
			if (shot->nChannels==1)
				cvCvtColor(shot, text, CV_GRAY2BGR);
			else
				cvCopy(shot, text);
		}

		// display text / react to keyboard
		switch (state) {
			default:
			case TAKE_SHOT:
				if (k==' ') {
					if (shot) cvCopy(frame,shot);
					else shot = cvCloneImage(frame);
					pause = true;
					state = CORNERS;
					k=-1;
				} else {
					putText(text,"Please take a frontal view", cvPoint(3,20), &font);
					putText(text,"of a textured planar surface", cvPoint(3,40), &font);
					putText(text,"and press space", cvPoint(3,60), &font);
					break;
				}
			case CORNERS:
					putText(text, "Drag corners to match the", cvPoint(3,20), &font);
					putText(text, "calibration target", cvPoint(3,40), &font);
					putText(text, "press 'r' to restart", cvPoint(3,60), &font);
					putText(text, "press space when ready", cvPoint(3,80), &font);
					if (k=='r') {
						pause = false;
						state = TAKE_SHOT;
					}
					if (k==' ') {
						accepted=true;
					}
					int four = 4;
					CvPoint *ptr = corners;
					cvPolyLine(text, &ptr, &four, 1, 1,
							cvScalar(0,255,0));
					break;
		}
		cvShowImage(win, text);
	}

	cvReleaseImage(&text);
	image = shot;


	return true;
}

void CalibModel::onMouseStatic(int event, int x, int y, int flags, void* param)
{
	if (param)
		((CalibModel *)param)->onMouse(event,x,y,flags);
	if (objectPtr)
		objectPtr->onMouse(event,x,y,flags);
	else
		cerr << "onMouseStatic(): null-pointer.\n";
}


