/*
 *  artvertiser.h
 *  artvertiser
 *
 *  Created by damian on 06/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "ofMain.h"
#include <cv.h>
#include "FTime.h"
#include "ofxControlPanel.h"

class Artvertiser
{
public:
	Artvertiser();
	~Artvertiser();
	
	void setup( int argc, char** argv );
	void update();
	void draw();
	
	/// clean things up on exit
	void exitHandler();
	
	void keyPressed (int key );
	void keyReleased( int key );
	void mouseMoved( int x , int y );
	void mouseDragged( int x, int y, int button );
	void mousePressed( int x, int y, int button );
	void mouseReleased( int x, int y, int button );
	
private:
	// draw the menu
	void drawMenu();
	
	// draw the current augmentation, if any
	void drawAugmentation();
	
	ofTrueTypeFont font_12;
	ofTrueTypeFont font_16;
	ofTrueTypeFont font_24;
	ofTrueTypeFont font_32;
	

	// delay buffer for frame delay
	list< pair<IplImage*, FTime> > frameRingBuffer;


	guiTypePanel* main_panel;
	guiTypeLabel* current_modelfile_label;
	guiTypeToggle* add_model_toggle;
	guiTypeToggle* retrain_current_toggle;
	guiTypeTextDropDown* model_selection_dropdown;
	ofxControlPanel control_panel;
	float control_panel_timer;
	
	void updateModelSelectionDropdown();
	
};

