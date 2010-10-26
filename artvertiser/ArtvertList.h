/*
 *  ArtvertList.h
 *  artvertiser
 *
 *  Created by damian on 26/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxThread.h"
#include "Artvert.h"

class ArtvertList
{
public:
	void setup();
	
	void loadFromXml( ofxXmlSettings& data );



private:
	
	vector< Artvert > artverts;

	ofxMutex lock;



};

