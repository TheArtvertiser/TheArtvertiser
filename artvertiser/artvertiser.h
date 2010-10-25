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
#include "Artvert.h"

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

	void windowResized( int new_w, int new_h ) { control_panel.setSize( new_w-10, new_h-40 ); }
private:
	// draw the men
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

	guiTypeTextDropDown* model_selection_dropdown;
	guiTypeLabel* model_status_label;

	guiTypeLabel* current_modelfile_label;
	guiTypeDrawable* current_modelfile_image_drawer;
	ofImage current_modelfile_image;
	guiTypeToggle* retrain_current_toggle;
	guiTypeToggle* add_model_toggle;
	guiTypeLabel* model_name_label;
	guiTypeToggle* retrain_geometry_toggle;
	
	guiTypeLabel* current_artvertfile_label;
	guiTypeDrawable* current_artvertfile_image_drawer;
	ArtvertDrawer current_artvert_drawer;
	guiTypeLabel* artvert_title_label;
	guiTypeTextInput* artvert_title_input;
	guiTypeLabel* artvert_artist_label;

	ofxControlPanel control_panel;
	float control_panel_timer;
	
	void updateModelSelectionDropdown();
	
};

