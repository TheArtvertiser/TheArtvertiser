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
#include "ofx3DModelLoader.h"

class Artvert
{
public:
	
	Artvert();
	~Artvert();
	
	void activate();
	void deactivate();
	bool isActive() { return active; }
	
	void shutdown();
	
	/// draw the artvert using the given set of corners (listed as x,y in clockwise order from topleft)
	void drawArtvert( float fade, const vector<float>& corners, bool for_preview = false );

	/// if skip_data_path_stuff is true, don't do an ofToDataPath processing on the path f
	void setModelFile( string f, bool skip_data_path_stuff=false ) { model_file = skip_data_path_stuff ? f : ofToDataPath( f ); }
	/// programatically determine the file type
	void changeArtvertFile( string new_file );
	void setArtvertImageFile( string f ) { assert(!active); artvert_image_file = ofToDataPath( f ); type = AV_IMAGE; }
	void setArtvertMovieFile( string f ) { assert(!active); artvert_movie_file = ofToDataPath( f ); type = AV_MOVIE; }
	void setArtvert3dsFile( string f )	 { assert(!active); artvert_3ds_file =	 ofToDataPath( f ); type = AV_3DS; }
	
	string getModelFile() { return model_file; }
	string getArtvertFile();
	string getArtvertImageFile() { return artvert_image_file; }
	string getArtvertMovieFile() { return artvert_movie_file; }
	string getArtvert3dsFile()	 { return artvert_3ds_file; }
	
	string getTitle() { return title; }
	void setTitle( string t ) { title = t; }
	string getArtist() { return artist; }
	void setArtist( string a ) { artist = a; }

	string getAdvertName() { return advert_name; }
	void setAdvertName( string n ) { advert_name = n; }
	
	bool artvertIsMovie() { return type == AV_MOVIE; }
	void setVolume( float vol );
	
	string getDescription();

	static void loadArtvertsFromXml( ofxXmlSettings& data, vector<Artvert*>& results );
	/// save the artvert and (optionally) model file data to xml. if save_model is true save the model, otherwise jsut save the artvert
	static void saveArtvertToXml( ofxXmlSettings& data, Artvert* artvert_to_save, bool save_model = true );
	
	typedef enum _ArtvertType{ AV_NONE, AV_IMAGE, AV_MOVIE, AV_3DS } ArtvertType;
	ArtvertType getArtvertType() { return type; }
	
private:
	string artist;
	string advert_name;
	string title;
	
	ArtvertType type;

	string model_file;
	string artvert_image_file;
	string artvert_movie_file;
	string artvert_3ds_file;
	
	
	ofVideoPlayer* avi_capture;

	int which_fallback_image;
	static vector<ofImage*> fallback_artvert_images;
	
	friend class ArtvertDrawer;
	
	ofx3DModelLoader model_loader;
	
	ofImage* fallback();
	
	bool active;
	
	ofTexture texture;
};



class ArtvertDrawer : public ofBaseDraws 
{
public:
	ArtvertDrawer() { artvert = NULL; }
	
	void useArtvert( Artvert* the_artvert ) { artvert = the_artvert; local_image.clear(); }
	
	
	void draw( float x, float y ) { draw( x,y,getWidth(),getHeight() ); }
	void draw( float x, float y, float width, float height );
	float getWidth() { return local_image.getWidth(); }
	float getHeight() { return local_image.getHeight(); }
	
private:
	
	ofImage local_image;
	Artvert* artvert;
	
};

