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
	void saveToXml( string xml_path );

	vector<string> getDescriptions();
	
	int getNumArtverts() { int result; lock(); result = artvert_list.size(); unlock(); return result; }
	
private:
	void lock() { mutex.lock(); }
	void unlock() { mutex.unlock(); }
	bool tryLock() { return mutex.tryLock(); }
	
	
	vector< Artvert* > artvert_list;

	ofxMutex mutex;

	bool needs_saving;


};

