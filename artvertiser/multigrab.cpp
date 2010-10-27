/*
 Copyright 2008, 2009, 2010 Julian Oliver <julian@julianoliver.com> 
 and Damian Stewart <damian@frey.co.nz>, based on BazAR which is 
 Copyright 2005, 2006 Computer Vision Lab, 3 Ecole Polytechnique 
 Federale de Lausanne (EPFL), Switzerland.
 Distributed under the terms of the GNU General Public License v3.
 
 This file is part of The Artvertiser.
 
 The Artvertiser is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The Artvertiser is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with The Artvertiser.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "multigrab.h"

#include "FProfiler/FProfiler.h"


MultiGrab::~MultiGrab()
{
    printf("in ~MultiGrab\n");
    // delete all the cameras;
    for ( int i=0; i<cams.size(); i++ )
    {
        delete cams[i];
    }
    cams.clear();

}

int MultiGrab::init( const char *avi_bg_path, int width, int height, int v4l_device, int detect_width, int detect_height, int desired_capture_fps )
{

	ofBaseVideo *c;
	cams.clear();

	// if artvertiser is presented an argv of a background file (-b /path/to/avi), accept it as a capture device.

	if (strlen(avi_bg_path) > 0)
	{
		cout << "MultiGrab::init creating capture from avi " << avi_bg_path << " size " << width << "x" << height << " detect at " << detect_width << "x" << detect_height << endl;
		ofVideoPlayer* player = new ofVideoPlayer();
		player->loadMovie( avi_bg_path );
		player->setLoopState( OF_LOOP_NORMAL );
		player->play();
		player->update();
		printf("player has new frame? %c\n", player->isFrameNew()?'y':'n' );
		ofBaseVideo *c = player;
		cams.push_back(new Cam(true, c, width, height, detect_width, detect_height, desired_capture_fps ));
	}
	// else capture from the V4L2 device
	else
	{
		/*
		while (1) {
			ofBaseVideo *c = ofBaseVideoFromCAM(cams.size());
			if (!c) break;
			cams.push_back(new Cam(c));
		}
		*/
		cout << "MultiGrab::init creating camera with device "<<v4l_device<<", capture at " << width << "x" << height << " detect at " << detect_width << "x" << detect_height << endl;
		ofVideoGrabber* grabber = new ofVideoGrabber();
		grabber->setDeviceID(v4l_device);
		if (!grabber->initGrabber( width, height, false ) )
		{
			printf( "couldn't initialise ofVideoGrabber\n");
			delete grabber;
			return 0;
		}
		grabber->update();
		
		ofBaseVideo *c = grabber;
		cams.push_back(new Cam(false, c, width, height, detect_width, detect_height, desired_capture_fps ));
	}
	if (cams.size()==0) {
		 return 0;
	}

    return 1;
}

void MultiGrab::clear()
{
	cams[0]->detector.clear();
}

bool MultiGrab::loadOrTrainCache( bool wants_training, const char* modelfile, bool train_on_binoculars )
{
    cams[0]->detector.clear();

    model.useModelFile( modelfile ) ;

	if (!model.buildCached(cams.size(), cams[0]->cam, !wants_training, cams[0]->detector, train_on_binoculars)) {
		cout << "model.buildCached() failed.\n";
		return false;
	}
	for (int i=1; i<cams.size(); ++i) {
		//! TODO mem to mem copy from cams[0]
		cams[i]->detector.load(string(modelfile)+".bmp.classifier");
	}

	return true;
}

void MultiGrab::allocLightCollector()
{
	for (vector<Cam *>::iterator it=cams.begin(); it!=cams.end(); ++it)
		(*it)->lc = new LightCollector(model.map.reflc);
}
void MultiGrab::Cam::setCam(ofBaseVideo *c, int _width, int _height, int _detect_width, int _detect_height, int desired_capture_fps, bool is_avi )
{

	if (cam)
	{
	    if ( mtc )
            delete mtc;
        mtc = 0;
		delete cam;
	}
	if (c==0) return;
	cam = c;

	if ( !is_avi )
	{
/*		// avoid saturating the firewire bus
		printf("setting fps to %i\n", desired_capture_fps );
		cvSetCaptureProperty(cam, CV_CAP_PROP_FPS, desired_capture_fps);
		cout << "setting cv capture property for size to " << _width << "x" << _height << endl;
		int res1 = cvSetCaptureProperty(cam, CV_CAP_PROP_FRAME_WIDTH, _width);
		int res2 = cvSetCaptureProperty(cam, CV_CAP_PROP_FRAME_HEIGHT, _height);
		printf("cvSetCaptureProperty: results %i %i\n", res1, res2 );*/
	}
	// optionally downsample the video to accelerate computation
    detect_width = _detect_width;
    detect_height = _detect_height;

	/*
	printf("cvGetCaptureProperty gives: %i %i\n", (int)cvGetCaptureProperty(cam, CV_CAP_PROP_FRAME_WIDTH),
			(int)cvGetCaptureProperty(cam, CV_CAP_PROP_FRAME_HEIGHT ) );

	printf("calling cvQueryFrame...\n");
	IplImage* test_image = cvQueryFrame( cam );
	printf("test_image was %lx\n", (unsigned long)test_image );
	if ( test_image != 0 )
	{
		printf(" width %i, height %i, depth %i, channels %i\n", test_image->width, test_image->height, test_image->depth, test_image->nChannels );
	}*/

   // now mtc
    mtc = new MultiThreadCapture( cam );
    mtc->setupResolution( detect_width, detect_height, /*grayscale*/ 1, desired_capture_fps );
    mtc->startCapture();
    IplImage* f = NULL;
    int timeout = 20*1000;
    printf("MultiGrab::Cam::setCam waiting for camera to become ready... (20s timeout)\n");
    bool got = false;
    while ( timeout >= 0 && !got )
    {
        // 50ms jumps
        usleep( 50*1000 );
        timeout -= 50;
        got = mtc->getCopyOfLastFrame( &f );
    }
	assert(f != 0 && "camera did not become ready");

	width = f->width; //cvRound(cvGetCaptureProperty(cam, CV_CAP_PROP_FRAME_WIDTH));
	height = f->height; //cvRound(cvGetCaptureProperty(cam, CV_CAP_PROP_FRAME_HEIGHT));
	cout << "Camera capturing " << width << "x" << height << " at "
		<< desired_capture_fps << " fps, " << f->nChannels << " channels.\n"
		" Detecting at "<< detect_width <<"x" << detect_height << endl;

	cvReleaseImage(&f);

}


MultiGrab::Cam::~Cam() {
    // gray is owned by mtc
	//if (gray && gray != frame) cvReleaseImage(&gray);
	if (frame_detectsize && frame_detectsize != gray && frame_detectsize != frame ) cvReleaseImage(&frame_detectsize);
	if (mtc) delete mtc;
	//if (cam) cvReleaseCapture(&cam);
	if (cam) delete cam;
	if (lc) delete lc;
}

void MultiGrab::Cam::shutdownMultiThreadCapture()
{
    if ( mtc )
        mtc->stopCapture();
}

// this could be run in a separate thread
bool MultiGrab::Cam::detect( bool &frame_retrieved, bool &detect_succeeded )
{
    PROFILE_THIS_FUNCTION();
	CvSize detect_size = cvSize( detect_width, detect_height );

	PROFILE_SECTION_PUSH("frame management");
    PROFILE_SECTION_PUSH("retrieve");
    IplImage *gray_temp, *frame_temp;
    FTime timestamp;
    bool got = mtc->getLastDetectFrame( &gray_temp, &frame_temp, &timestamp,/*block until available*/ false );
    PROFILE_SECTION_POP();
    if ( !got || frame_temp == 0 || gray_temp == 0 )
    {
        PROFILE_SECTION_POP();
        frame_retrieved = false;
        return false;
    }
    frame_retrieved = true;
    gray = gray_temp;
    frame = frame_temp;
    detected_frame_timestamp = timestamp;
    PROFILE_SECTION_POP();


    if ( !detector.isReady() )
    {
		//printf("detector is not ready, detect failed\n");
        detect_succeeded = false;
        return false;
    }

	PROFILE_SECTION_PUSH( "detection" );



	// run the detector
	bool res = false;
	detector.lock();
	res = detector.detect(gray);
    detect_succeeded = detector.object_is_detected;
	detector.unlock();
	if (res) {
	    PROFILE_SECTION_PUSH("light accumulator")
		if (lc)
            lc->averageImage(frame, detector.H);
        PROFILE_SECTION_POP();
	}

	PROFILE_SECTION_POP();

    return res;
}

bool add_detected_homography(int n, planar_object_recognizer &detector, CamCalibration &calib)
{
	static std::vector<CamCalibration::s_struct_points> pts;
	pts.clear();

	for (int i=0; i<detector.match_number; ++i) {
			image_object_point_match * match = detector.matches+i;
			if (match->inlier) {
				pts.push_back(CamCalibration::s_struct_points(
					PyrImage::convCoordf(match->image_point->u, int(match->image_point->scale), 0),
					PyrImage::convCoordf(match->image_point->v, int(match->image_point->scale), 0),
					PyrImage::convCoordf((float)match->object_point->M[0], int(match->object_point->scale), 0),
					PyrImage::convCoordf((float)match->object_point->M[1], int(match->object_point->scale), 0)));
			}
	}

	return calib.AddHomography(n, pts, detector.H);
}

bool add_detected_homography(int n, planar_object_recognizer &detector, CamAugmentation &a)
{
	static std::vector<CamCalibration::s_struct_points> pts;
	pts.clear();

	for (int i=0; i<detector.match_number; ++i) {
			image_object_point_match * match = detector.matches+i;
			if (match->inlier) {
				pts.push_back(CamCalibration::s_struct_points(
					PyrImage::convCoordf(match->image_point->u, int(match->image_point->scale), 0),
					PyrImage::convCoordf(match->image_point->v, int(match->image_point->scale), 0),
					PyrImage::convCoordf(match->object_point->M[0], int(match->object_point->scale), 0),
					PyrImage::convCoordf(match->object_point->M[1], int(match->object_point->scale), 0)));
			}
	}

	a.AddHomography(pts, detector.H);
	return true;
}
