/*
 *  Artvert.h
 *  artvertiser
 *
 *  Created by damian on 19/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#pragma once
#include "ofMain.h"
#include <cv.h>
#include "avImage.h"

#include "ofxXmlSettings.h"

class Artvert
{
public:
	
	Artvert();
	~Artvert();
	
	void activate();
	void deactivate();
	
	IplImage* getArtvertImage();
	
	bool artvert_is_movie;

	/// if skip_data_path_stuff is true, don't do an ofToDataPath processing on the path f
	void setModelFile( string f, bool skip_data_path_stuff=false ) { model_file = skip_data_path_stuff ? f : ofToDataPath( f ); }
	void setArtvertImageFile( string f ) { artvert_image_file = ofToDataPath( f ); }
	void setArtvertMovieFile( string f ) { artvert_movie_file = ofToDataPath( f ); }
	
	string getModelFile() { return model_file; }
	string getArtvertFile() { if ( artvert_is_movie ) return getArtvertMovieFile(); else return getArtvertImageFile(); }
	string getArtvertImageFile() { return artvert_image_file; }
	string getArtvertMovieFile() { return artvert_movie_file; }
	
	string getTitle() { return title; }
	void setTitle( string t ) { title = t; }
	string getArtist() { return artist; }
	void setArtist( string a ) { artist = a; }

	string getAdvertName() { return advert_name; }
	void setAdvertName( string n ) { advert_name = n; }
	
	bool artvertIsMovie() { return artvert_is_movie; }
	void setVolume( float vol );
	
	string getDescription();

	static void loadArtvertsFromXml( ofxXmlSettings& data, vector<Artvert>& results );
	static void saveArtvertToXml( ofxXmlSettings& data, Artvert& artvert_to_save );
	
private:
	string artist;
	string advert_name;
	string title;

	string model_file;
	string artvert_image_file;
	string artvert_movie_file;
	ofVideoPlayer* avi_capture;
	IplImage* avi_image;
	bool avi_load_attempted;
	bool avi_storage_initialised;
	GLuint imageID;
	
	IplImage* artvert_image;

	int which_fallback_image;
	static vector<IplImage*> fallback_artvert_images;
	
	friend class ArtvertDrawer;
	
	IplImage* fallback();
	
};



class ArtvertDrawer : public ofBaseDraws 
{
public:
	ArtvertDrawer() { artvert = NULL; }
	
	void useArtvertList( vector<Artvert>* the_list ) { artvert_list = the_list; }
	void useArtvert( int the_artvert ) { artvert = the_artvert; local_image.clear(); }
	
	
	void draw( float x, float y ) { draw( x,y,getWidth(),getHeight() ); }
	void draw( float x, float y, float width, float height );
	float getWidth() { return local_image.getWidth(); }
	float getHeight() { return local_image.getHeight(); }
	
private:
	
	ofImage local_image;
	vector<Artvert>* artvert_list;
	int artvert;
	
};

