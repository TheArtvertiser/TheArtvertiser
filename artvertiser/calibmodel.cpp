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
 
#include "calibmodel.h"
#include "multigrab.h"
#include "avImage.h"

char CalibModel::progress_string[2048];

CalibModel::CalibModel()
{
	image=0;
	image_width = 0;
	image_height = 0;
	win = "The Artvertiser 0.4";
	train_working_image = 0;
	interactive_train_running = false;
	interactive_train_should_stop = false;
	debounce_green = false;
	debounce_redblue = false;
	strcpy(progress_string, "");
	learn_running = false;

	cvInitFont(&train_font, CV_FONT_HERSHEY_SIMPLEX, .5, .5, 0, 0, CV_AA);
}

CalibModel::~CalibModel()
{
	if (image) cvReleaseImage(&image);
	if ( train_working_image )
    {
        cvReleaseImage( &train_working_image );
        cvReleaseImage( &train_shot );
    }
}

void CalibModel::useModelFile(const char* file)
{
    modelfile = file;
}

CalibModel *objectPtr=0;

/*void CalibModel::mousePressed( int x, int y )
{
    CvPoint* ptr;
    if ( state == ARTVERT_CORNERS )
        ptr = artvert_corners;
    else
        ptr = corners;
	
	// try to grab something
	grab = -1;
	for (int i=0; i<4; i++)
	{
		int dx = x-ptr[i].x;
		int dy = y-ptr[i].y;
		if (sqrt((double)(dx*dx+dy*dy)) <10) {
			grab = i;
			break;
		}
	}

	if ( grab != -1 )
	{
		ptr[grab].x = x;
		ptr[grab].y = y;
	}
	
}

void CalibModel::mouseReleased() 
{
	grab = -1;
}

void CalibModel::mouseDragged( int x, int y )
{
	
	if ( grab != -1 )
	{
		CvPoint* ptr;
		if ( state == ARTVERT_CORNERS )
			ptr = artvert_corners;
		else
			ptr = corners;
		ptr[grab].x = x;
		ptr[grab].y = y;
	}
}
*/
bool CalibModel::buildCached(int nbcam, ofBaseVideo *capture,
                             bool cache, planar_object_recognizer &detector,
                             bool running_on_binoculars )
{

    detector.clear();

	printf("buildCached about to grab a lock\n");

    detector.lock();


	detector.ransac_dist_threshold = 5;
	detector.max_ransac_iterations = 800;
	//detector.ransac_stop_support = 50;
	//detector.non_linear_refine_threshold = 15.0f;
	//detector.point_detector_tau = 10;

	// A lower threshold will allow detection in harder conditions, but
	// might lead to false positives.
	detector.match_score_threshold=.03f;

	detector.min_view_rate=.1;
	detector.views_number = 1000;
	// damian below
   //detector.min_view_rate = .2;

    static const int MAX_MODEL_KEYPOINTS = 500;     // maximum number of keypoints on the model
    static const int PATCH_SIZE = 32;               // patch size in pixels
    static const int YAPE_RADIUS = 5;               // yape radius
    static const int NUM_TREES = 12;                // num classifier trees
    static const int NUM_GAUSSIAN_LEVELS = 3;       // num gaussian levels
    /*static const int MAX_MODEL_KEYPOINTS = 500;     // maximum number of keypoints on the model
    static const int PATCH_SIZE = 32;               // patch size in pixels
    static const int YAPE_RADIUS = 5;               // yape radius
    static const int NUM_TREES = 12;                // num classifier trees
    static const int NUM_GAUSSIAN_LEVELS = 3;       // num gaussian levels*/


	// Should we train or load the classifier ?
	if ( cache )
        printf("model.buildCached() trying to load from %s...\n", modelfile );
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

        MultiThreadCapture* mtc = MultiThreadCaptureManager::getInstance()->getCaptureForCam(capture);
        /*
        IplImage* init_image = NULL;
        int timeout = 10000;
        bool got = false;
        do {
            got = mtc->getCopyOfLastFrame( &init_image );
        }
        while ( !got &&
               !usleep( 10*1000 ) &&
               (timeout-=10) > 0 );
        if ( init_image == NULL )
        {
            printf("getCopyOfLastFrame timed out: capture failed\n");
            detector.unlock();
            detector.clear();
            return false;
        }*/
        if ( image != 0 )
            cvReleaseImage( &image );
		image = avLoadImage(modelfile, mtc->getNumChannelsRaw()==3 );
		image_width = image->width;
		image_height = image->height;
		//cvReleaseImage(&init_image);

        /*printf("dumping loaded cache:\n");
        detector.dump();
        */

	}
	else
	{
        if ( image != 0 )
            cvReleaseImage( &image );
        image =0;
		// ask the user the take a shot of the model
		bool result = interactiveTrain();
		if (!result)
		{
		    printf("interactiveTrain failed\n");
		    detector.unlock();
		    detector.clear();
            return false;
		}
		else
		{
			printf("interactiveTrain succeeded\n");
		}

        image_width = image->width;
		image_height = image->height;

		// train the classifier to detect this model
		//if (!detector.build(image, 500, 32, 3, 12, 3,0, 0))
		//if (!detector.build(image, 200, 32, 5, 20, 3,0, 0))
		int working_roi[8] = { corners[0].x, corners[0].y,
            corners[1].x, corners[1].y,
            corners[2].x, corners[2].y,
            corners[3].x, corners[3].y
            };
        //LEARNPROGRESSION progress;
		//LEARNPROGRESSION progress = 0;

		printf("about to call detector.build...\n");
		learn_running = true;
		strcpy(progress_string, "preparing..");	
		if (!detector.build(image,
                MAX_MODEL_KEYPOINTS, // max keypoints
				PATCH_SIZE, // patch size
				YAPE_RADIUS,                 // yape radius. Use 3,5 or 7.
				NUM_TREES,                // number of trees for the classifier. Somewhere between 12-50
				NUM_GAUSSIAN_LEVELS,      // number of levels in the gaussian pyramid
				&learnProgressionFunc,
				working_roi
				))
        {
			learn_running = false;
		    printf("build based on interactiveSetup failed\n");
			detector.unlock();
			detector.clear();
			return false;

		}
		printf("detector.build succeeded\n");
		strcpy( progress_string, "saving..\n");
		// save the image
		if (!avSaveImage(modelfile, image))
		{
		    printf("saving input image failed\n");
			learn_running = false;
		    detector.unlock();
		    detector.clear();
		    return false;
		}

		// and the region of interest (ROI)
		string roifn = string(modelfile) + ".roi";
		ofstream roif(roifn.c_str());
		if (!roif.good())
		{
			learn_running = false;
		    detector.unlock();
		    detector.clear();
 		    printf("saving .roi file failed\n");
            return false;
		}
		for (int i=0;i<4; i++)
			roif << corners[i].x << " " << corners[i].y << "\n";
		roif.close();

		// and the artvert corners
 		roifn = string(modelfile) + ".artvertroi";
		ofstream artvert_roif(roifn.c_str());
		if (!artvert_roif.good())
		{
			learn_running = false;
		    detector.unlock();
		    detector.clear();
 		    printf("saving .artvertroi file failed\n");
            return false;
		}
		for (int i=0;i<4; i++)
			artvert_roif << artvert_corners[i].x << " " << artvert_corners[i].y << "\n";
		artvert_roif.close();

		// and the trained classifier
		string classifier_directory = string(modelfile)+".classifier";
		detector.save(classifier_directory);

        string stable_points_filename = string(modelfile)+"_stable_points.bmp";
        printf("saving stable points to %s\n", stable_points_filename.c_str());
        detector.save_image_of_model_points(PATCH_SIZE, stable_points_filename.c_str() );

        const char* initial_points_filename = "initial_model_points.bmp";
        string initial_points_new_filename = string(modelfile)+"_initial_points.bmp";
        printf("renaming %s to %s\n", initial_points_filename, initial_points_new_filename.c_str() );
        rename(initial_points_filename, initial_points_new_filename.c_str() );

        /*printf("dumping trained cache:\n");
        detector.dump();*/

		learn_running = false;
	}

	detector.unlock();
	assert( detector.isReady() );

	float cn[4][2];
	for (int i=0; i<4; i++)
	{
		cn[i][0] = corners[i].x;
		cn[i][1] = corners[i].y;
        cout << corners[i].x << " " << corners[i].y << endl;
	}

	// prepare the light calibration reference
	return map.init(nbcam, image, cn, 8, 6);
}

static void putText(IplImage *im, const char *text, CvPoint p, CvFont *f1)
{
	cvPutText(im,text,p,f1, cvScalar(0,255, 255));
}



bool CalibModel::interactiveTrain()
{
    // setup
    state = TAKE_SHOT;
    train_should_proceed = false;
    // we want the interactive training to run
    interactive_train_running = true;
	interactive_train_should_stop = false;
	debounce_green = true;
	debounce_redblue = true;

    // wait until done
    int timeout =5*60;
    // 5 minute time out
    while ( interactive_train_running && !interactive_train_should_stop && timeout>0 )
    {
        printf("waiting for interactiveTrainBinoculars to complete .. %i\n", timeout );
        timeout-=5;
        sleep(5);
    }

    return train_should_proceed;
}

void CalibModel::interactiveTrainUpdateBinoculars( IplImage* frame,
                                                  bool button_red,
                                                  bool button_green,
                                                  bool button_blue )
{
    if ( !interactive_train_running )
        return;

	if ( interactive_train_should_stop )
	{
		interactive_train_running =  false;
		return;
	}

    // copy to training
    if ( train_working_image == 0 )
    {
        train_working_image = cvCreateImage( cvGetSize( frame ), frame->depth, frame->nChannels );
        train_shot = cvCreateImage( cvGetSize( frame ), frame->depth, frame->nChannels );
    }

    // inputs
    bool R__ = ( button_red&&!button_green&&!button_blue);
    bool _G_ = (!button_red&& button_green&&!button_blue);
    bool __B = (!button_red&&!button_green&& button_blue);
    bool RG_ = ( button_red&& button_green&&!button_blue);
    bool _GB = (!button_red&& button_green&& button_blue);
    bool R_B = ( button_red&&!button_green&& button_blue);
    bool RGB = ( button_red&& button_green&& button_blue);

	// debounce 
	
	// don't allow just green if we're trying to debounce
	if ( !button_green )
		debounce_green = false;
	else if ( debounce_green && _G_ )
		_G_ = false;
	
	// don't allow red+blue if we're trying to debounce
	if ( !button_red || !button_blue )
		debounce_redblue = false;
	else if ( debounce_redblue && R_B )
		R_B = false;

    // for drawing
    int four = 4;
    CvPoint *ptr;
	int oldx, oldy;

    // update
    switch( state )
    {
    case TAKE_SHOT:
        if ( _G_ )
        {
            // advance
            cvCopy( frame, train_shot );
            state = CORNERS;
            int d = 30;
            corners[0].x = d;
            corners[0].y = d;
            corners[1].x = frame->width-d;
            corners[1].y = d;
            corners[2].x = frame->width-d;
            corners[2].y = frame->height-d;
            corners[3].x = d;
            corners[3].y = frame->height-d;
            // setup index
            corner_index=0;
            x = corners[corner_index].x;
            y = corners[corner_index].y;
			// debounce the green button
			debounce_green = true;
        }
        else if ( R_B )
        {
            // abort
            interactive_train_running = false;
			debounce_redblue = true;
        }
        else
        {
            cvCopy( frame, train_working_image );
            putText(train_working_image, modelfile, cvPoint(3,20), &train_font );
			putText(train_working_image,"Please take a frontal view", cvPoint(3,40), &train_font);
			putText(train_working_image,"of a textured planar surface", cvPoint(3,60), &train_font);
			putText(train_working_image,"and press green", cvPoint(3,80), &train_font);
			putText(train_working_image,"Press red+blue to abort", cvPoint(3,100), &train_font);
        }
        break;
    case CORNERS:
        cvCopy( train_shot, train_working_image );
        putText(train_working_image, modelfile, cvPoint(3,20), &train_font);
		putText(train_working_image, "Move green corners to match the", cvPoint(3,40), &train_font);
		putText(train_working_image, "calibration target", cvPoint(3,60), &train_font);
		putText(train_working_image, "Press red+blue to restart", cvPoint(3,80), &train_font);
		putText(train_working_image, "Press green when ready", cvPoint(3,100), &train_font);

		oldx=x; oldy=y;
        if      ( R__ )
            x += 1;
        else if ( __B )
            x -= 1;
        else if ( RG_ )
            y += 1;
        else if ( _GB )
            y -= 1;
		// update corner if necessary
		if ( oldx != x )
			corners[corner_index].x = x;
		if ( oldy != y )
			corners[corner_index].y = y;
		// continue checking buttons
        if ( _G_ )
        {
            // accept
            corners[corner_index].x = x;
            corners[corner_index].y = y;
            corner_index++;
            if ( corner_index > 3 )
            {
                // next
                corner_index = 0;
                for ( int i=0; i<4; i++ )
                    artvert_corners[i] = corners[i];
                state = ARTVERT_CORNERS;

				// setup index
				corner_index=0;
				x = artvert_corners[corner_index].x;
				y = artvert_corners[corner_index].y;
			}
			else
			{
				x = corners[corner_index].x;
				y = corners[corner_index].y;
			}	

			debounce_green = true;
		}
		else if ( R_B )
		{
			// back
			corner_index--;
			if ( corner_index < 0 )
				state = TAKE_SHOT;
			else
			{
				x = corners[corner_index].x;
				y = corners[corner_index].y;
			}
			debounce_redblue = true;
		}

		ptr = corners;
		cvPolyLine(train_working_image, &ptr, &four, 1, 1,
				cvScalar(0,255,0));
		cvCircle( train_working_image, corners[corner_index], 10, cvScalar(0,255,0));

		break;

	case ARTVERT_CORNERS:
		cvCopy( train_shot, train_working_image );
		putText(train_working_image, modelfile, cvPoint(3,20), &train_font);
		putText(train_working_image, "Move red corners to match the", cvPoint(3,40), &train_font);
		putText(train_working_image, "artvert target area;", cvPoint(3,60), &train_font);
		putText(train_working_image, "Press red+blue to restart", cvPoint(3,80), &train_font);
		putText(train_working_image, "Press green when ready", cvPoint(3,100), &train_font);

		oldx = x; oldy = y;
		if ( R__ )
			x += 1;
		else if ( __B )
			x -= 1;
		else if ( RG_ )
			y += 1;
		else if ( _GB )
			y -= 1;
		// update corner if necessary
		if ( oldx != x )
			artvert_corners[corner_index].x = x;
		if ( oldy != y )
			artvert_corners[corner_index].y = y;
		// continue checking buttons
		if ( _G_ )
		{
			// accept
			artvert_corners[corner_index].x = x;
			artvert_corners[corner_index].y = y;
			corner_index++;
			if ( corner_index > 3 )
			{
				// finished
				if ( image != 0 )
					cvReleaseImage( &image );
				image = cvCreateImage( cvGetSize( train_shot), train_shot->depth, train_shot->nChannels );
				cvCopy( train_shot, image );
				train_should_proceed = true;
				interactive_train_running = false;
			}
			else
			{
				x = artvert_corners[corner_index].x;
				y = artvert_corners[corner_index].y;
			}
			debounce_green = true;
		}
		else if ( R_B )
		{
			// back
			corner_index--;
			if ( corner_index < 0 )
				state = CORNERS;
			else
			{
				x = artvert_corners[corner_index].x;
				y = artvert_corners[corner_index].y;
			}
			debounce_redblue = true;

		}

		ptr = corners;
		cvPolyLine(train_working_image, &ptr, &four, 1, 1,
				cvScalar(0,255,0));
		ptr = artvert_corners;
		cvPolyLine(train_working_image, &ptr, &four, 1, 1,
				cvScalar(0,0,255));
		cvCircle( train_working_image, artvert_corners[corner_index], 10, cvScalar(0,0,255));

		break;
	}

	train_texture.setImage( train_working_image );

}

void CalibModel::interactiveTrainDraw()
{
	if ( !interactive_train_running )
		return;

	IplTexture* tex = &train_texture;

	IplImage *im = tex->getIm();
	int w = im->width-1;
	int h = im->height-1;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	tex->loadTexture();

	glBegin(GL_QUADS);
	glColor4f(1,1,1,1);

	glTexCoord2f(tex->u(0), tex->v(0));
	glVertex2f(-1, 1);
	glTexCoord2f(tex->u(w), tex->v(0));
	glVertex2f(1, 1);
	glTexCoord2f(tex->u(w), tex->v(h));
	glVertex2f(1, -1);
	glTexCoord2f(tex->u(0), tex->v(h));
	glVertex2f(-1, -1);
	glEnd();

	tex->disableTexture();

}




//////////////////////////////////////////////////////////////////////////////

void CalibModel::interactiveTrainUpdate( IplImage* frame, 
											  int mouse_x, int mouse_y, bool mouse_button_down, 
											  int key )
{
    if ( !interactive_train_running )
        return;
	
	if ( interactive_train_should_stop )
	{
		interactive_train_running =  false;
		return;
	}
	
    // copy to training
    if ( train_working_image == 0 )
    {
        train_working_image = cvCreateImage( cvGetSize( frame ), frame->depth, frame->nChannels );
        train_shot = cvCreateImage( cvGetSize( frame ), frame->depth, frame->nChannels );
    }
		
    // for drawing
    int four = 4;
    CvPoint *ptr;
	int oldx, oldy;
	float closest_distance;
	
    // update
    switch( state )
    {
		case TAKE_SHOT:
			if ( key == ' ' )
			{
				// advance
				cvCopy( frame, train_shot );
				state = CORNERS;
				int d = 30;
				corners[0].x = d;
				corners[0].y = d;
				corners[1].x = frame->width-d;
				corners[1].y = d;
				corners[2].x = frame->width-d;
				corners[2].y = frame->height-d;
				corners[3].x = d;
				corners[3].y = frame->height-d;
				// setup index
				corner_index=0;
				x = corners[corner_index].x;
				y = corners[corner_index].y;
			}
			else if ( key == 'r' )
			{
				// abort
				interactive_train_running = false;
				debounce_redblue = true;
			}
			else
			{
				cvCopy( frame, train_working_image );
				putText(train_working_image, modelfile, cvPoint(3,20), &train_font );
				putText(train_working_image,"Please take a frontal view", cvPoint(3,40), &train_font);
				putText(train_working_image,"of a textured planar surface", cvPoint(3,60), &train_font);
				putText(train_working_image,"and press space", cvPoint(3,80), &train_font);
				putText(train_working_image,"Press r to abort", cvPoint(3,100), &train_font);
			}
			break;
		case CORNERS:
			cvCopy( train_shot, train_working_image );
			putText(train_working_image, modelfile, cvPoint(3,20), &train_font);
			putText(train_working_image, "Move green corners to match the", cvPoint(3,40), &train_font);
			putText(train_working_image, "calibration target", cvPoint(3,60), &train_font);
			putText(train_working_image, "Press r to restart", cvPoint(3,80), &train_font);
			putText(train_working_image, "Press space when ready", cvPoint(3,100), &train_font);

			// work out which corner is closest to the mouse
			corner_index = -1;
			for ( int i=0; i<4; i++ )
			{
				float dx = (corners[i].x-mouse_x);
				float dy = (corners[i].y-mouse_y);
				float distance = sqrtf(dx*dx + dy*dy);
				if ( distance < 10.0f )
				{
					if ( corner_index == -1 || closest_distance > distance )
					{
						corner_index = i;
						closest_distance = distance;
					}
				}
					
			}
			
			if ( mouse_button_down && corner_index != -1 )
			{
				corners[corner_index].x = mouse_x;
				corners[corner_index].y = mouse_y;
			}
			
			if ( key == ' ' )
			{
				// accept
				for ( int i=0; i<4; i++ )
					artvert_corners[i] = corners[i];
				state = ARTVERT_CORNERS;
			}
			else if ( key == 'r' )
			{
				// back
				state = TAKE_SHOT;
			}
			
			ptr = corners;
			cvPolyLine(train_working_image, &ptr, &four, 1, 1,
					   cvScalar(0,255,0));
			if ( corner_index != -1 )
				cvCircle( train_working_image, corners[corner_index], 10, cvScalar(0,255,0));
			
			break;
			
		case ARTVERT_CORNERS:
			cvCopy( train_shot, train_working_image );
			putText(train_working_image, modelfile, cvPoint(3,20), &train_font);
			putText(train_working_image, "Move red corners to match the", cvPoint(3,40), &train_font);
			putText(train_working_image, "artvert target area;", cvPoint(3,60), &train_font);
			putText(train_working_image, "Press r to restart", cvPoint(3,80), &train_font);
			putText(train_working_image, "Press space when ready", cvPoint(3,100), &train_font);

			// work out which corner is closest to the mouse
			corner_index = -1;
			for ( int i=0; i<4; i++ )
			{
				float dx = (artvert_corners[i].x-mouse_x);
				float dy = (artvert_corners[i].y-mouse_y);
				float distance = sqrtf(dx*dx + dy*dy);
				if ( distance < 10.0f )
				{
					if ( corner_index == -1 || closest_distance > distance )
					{
						corner_index = i;
						closest_distance = distance;
					}
				}
				
			}
			
			if ( mouse_button_down && corner_index != -1 )
			{
				artvert_corners[corner_index].x = mouse_x;
				artvert_corners[corner_index].y = mouse_y;
			}
			
			if ( key == ' ' )
			{
				// finished
				if ( image != 0 )
					cvReleaseImage( &image );
				image = cvCreateImage( cvGetSize( train_shot), train_shot->depth, train_shot->nChannels );
				cvCopy( train_shot, image );
				train_should_proceed = true;
				interactive_train_running = false;
			}
			else if ( key == 'r' )
			{
				// back
				state = CORNERS;
			}
			
			ptr = corners;
			cvPolyLine(train_working_image, &ptr, &four, 1, 1,
					   cvScalar(0,255,0));
			ptr = artvert_corners;
			cvPolyLine(train_working_image, &ptr, &four, 1, 1,
					   cvScalar(0,0,255));
			if ( corner_index != -1 )
				cvCircle( train_working_image, artvert_corners[corner_index], 10, cvScalar(0,0,255));
			
			break;
	}
	
	train_texture.setImage( train_working_image );
	
}


/////////////////////////////////////////////////////////////////////

/*
bool interactive_win_created = false;
bool CalibModel::interactiveSetup(ofBaseVideo *capture )
{




	CvFont font, fontbold;

	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, .5, .5, 0, 0, CV_AA);

	const char* interactive_win = "The Artvertiser training window";

	
	if( !interactive_win_created )
	{
		cvNamedWindow(interactive_win, CV_WINDOW_AUTOSIZE);
		cvSetMouseCallback(interactive_win, onMouseStatic, this);
		interactive_win_created = true;
	}
	grab=-1;

	objectPtr = this;

	bool pause=false;

	IplImage *frame_gray, *frame = NULL;
	FTime timestamp;
	MultiThreadCapture* mtc = MultiThreadCaptureManager::getInstance()->getCaptureForCam(capture);
	int timeout = 10000;
	bool got = false;
	do {
		got = mtc->getLastDetectFrame( &frame_gray, &frame, &timestamp, true );
	}
	while ( !got &&
			!usleep( 100000 ) &&
			(timeout-=100) > 0 );
	if ( frame == NULL )
	{
		printf("capture failed\n");
		return false;
	}

	IplImage *shot=0, *text=0;

	state = TAKE_SHOT;

	bool accepted =false;
	bool artvert_accepted = false;
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
			bool got = mtc->getLastDetectFrame( &frame_gray, &frame, NULL,true );
			if ( !got )
				continue;
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

		int four = 4;
		CvPoint *ptr;
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
					putText(text, modelfile, cvPoint(3,20), &font);
					putText(text,"Please take a frontal view", cvPoint(3,40), &font);
					putText(text,"of a textured planar surface", cvPoint(3,60), &font);
					putText(text,"and press space", cvPoint(3,80), &font);
					break;
				}
			case CORNERS:
				putText(text, modelfile, cvPoint(3,20), &font);
				putText(text, "Drag green corners to match the", cvPoint(3,40), &font);
				putText(text, "calibration target", cvPoint(3,60), &font);
				putText(text, "press 'r' to restart", cvPoint(3,80), &font);
				putText(text, "press space when ready", cvPoint(3,100), &font);
				if (k=='r') {
					pause = false;
					state = TAKE_SHOT;
				}
				if (k==' ') {
					for ( int i=0;i<4; i++ )
					{
						artvert_corners[i].x = corners[i].x;
						artvert_corners[i].y = corners[i].y;
					}
					state = ARTVERT_CORNERS;
				}
				ptr = corners;
				cvPolyLine(text, &ptr, &four, 1, 1,
						cvScalar(0,255,0));
				break;
			case ARTVERT_CORNERS:
				putText(text, "Drag red corners to match the", cvPoint(3,20), &font);
				putText(text, "artvert target area;", cvPoint(3,40), &font);
				putText(text, "press 'r' to restart", cvPoint(3,60), &font);
				putText(text, "press space when ready", cvPoint(3,80), &font);
				if (k=='r') {
					pause = false;
					state = TAKE_SHOT;
				}
				if (k==' ') {
					accepted = true;
				}
				ptr = corners;
				cvPolyLine(text, &ptr, &four, 1, 1,
						cvScalar(0,255,0));
				ptr = artvert_corners;
				cvPolyLine(text, &ptr, &four, 1, 1,
						cvScalar(0,0,255));
				break;
		}
		cvShowImage(interactive_win, text);
	}

	cvReleaseImage(&text);
	image = shot;

	//cvDestroyWindow( interactive_win );
	// make sure the destroy window succeeds
	//cvWaitKey(0);

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
*/

void CalibModel::learnProgressionFunc( int phase, int current, int total )
{
	sprintf(progress_string, "learning, phase %i/4: %4.1f%%", phase+1, 100.0f*float(current)/total );
	if ( current==0 )
		printf("\n");
	printf("\rphase %i/4: %4i/%4i ", phase+1, current, total);
	fflush(stdout);

	/*for ( int i=0; i<4; i++ )
	{
		if ( phase > i )
			strcat(progress_string, "[----------------------------------------]\n" );
		else if ( phase == i )
		{
			float finishedPct = float(current)/float(total);
			int count = finishedPct*40;
			strcat(progress_string, "[");
			for ( int j=0; j<count; j++ )
			{
				strcat(progress_string, "-");
			}
			for ( int j=0; j<40-count; j++ )
			{
				strcat(progress_string, " ");
			}
			strcat(progress_string, "]\n");
		}
		else
			strcat( progress_string, "[                                        ]\n");
	}*/

}

