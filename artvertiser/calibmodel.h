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
 
#ifndef CALIBMODEL_H
#define CALIBMODEL_H

#include "ofMain.h"

#include <cv.h>
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

	bool buildCached(int nbcam, ofBaseVideo *capture, bool cache, planar_object_recognizer &detector,
                  bool run_on_binoculars = false);

    int getImageWidth() { return image_width; }
    int getImageHeight() { return image_height; }

    /// update the interactive training on binoculars
    void interactiveTrainUpdateBinoculars(IplImage* frame, bool button_red, bool button_green, bool button_blue );
	/// update the interactive training inline
    void interactiveTrainUpdate(IplImage* frame, 
									  int mouse_x, int mouse_y, bool mouse_button_down, 
									  int key );
	
    /// draw the interactive training on binucolurs
    void interactiveTrainDraw();
    /// true if we're running interactive training on the binoculars
    bool isInteractiveTrainRunning() { return interactive_train_running; }
	/// tell the interactive training to abort
	void abortInteractiveTrain() { if( interactive_train_running ) interactive_train_should_stop = true; }

		
	/// true if learning is in progress
	bool isLearnInProgress() { return learn_running; }
	/// get the training progress message
	const char* getLearnProgressMessage() { return progress_string; }

	// mouse
	/*
	void mousePressed( int x, int y );
	void mouseReleased();
	void mouseDragged( int x, int y );
	 */
	
private:
	IplImage *image;

	int image_width, image_height;

	const char *win;
	const char *modelfile;

	enum State { TAKE_SHOT, CORNERS, ARTVERT_CORNERS };
	State state;
	//int grab;
	//static void onMouseStatic(int event, int x, int y, int flags, void* param);
	//void onMouse(int event, int x, int y, int flags);
	//bool interactiveSetup(ofBaseVideo *capture);

	bool interactiveTrain();
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
