/*
 *  artvertiser.h
 *  artvertiser
 *
 *  Created by damian on 06/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "ofMain.h"

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
	
	
};

