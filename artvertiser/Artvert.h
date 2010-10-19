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

class Artvert
{
public:
	
	Artvert();
	~Artvert();
	
	IplImage* getArtvertImage();
	
	string artist;
	string advert;
	string name;
	bool artvert_is_movie;
	
	void setModelFile( string f, bool skip_data_path_stuff=false ) { model_file = skip_data_path_stuff ? f : ofToDataPath( f ); }
	void setArtvertImageFile( string f ) { artvert_image_file = ofToDataPath( f ); }
	void setArtvertMovieFile( string f ) { artvert_movie_file = ofToDataPath( f ); }
	
	string getModelFile() { return model_file; }
	string getArtvertImageFile() { return artvert_image_file; }
	string getArtvertMovieFile() { return artvert_movie_file; }
private:
	string model_file;
	string artvert_image_file;
	string artvert_movie_file;
	ofBaseVideo* avi_capture;
	IplImage* avi_image;
	bool avi_play_init;
	GLuint imageID;
	
	IplImage* artvert_image;

	int which_fallback_image;
	static vector<IplImage*> fallback_artvert_images;
	
};
