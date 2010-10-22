/*
 *  Artvert.cpp
 *  artvertiser
 *
 *  Created by damian on 19/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "Artvert.h"

vector<IplImage*> Artvert::fallback_artvert_images;

Artvert::Artvert()
{ 
	artvert_image=0; 
	model_file="<uninitialised>"; 
	artvert_image_file="<uninitialised>"; 
	artvert_is_movie= false;
	artist = "unknown artist";
	advert_name = "unknown advert";
	title = "untitled artvert";
	avi_capture = NULL;
	avi_image = NULL;
	avi_storage_initialised = false;
	
	printf("loading fallback artvert images from data/...\n");
	if ( fallback_artvert_images.size()==0 )
	{
		// load fallback images
		// default, should always be there
		IplImage* image = avLoadImage(ofToDataPath("fallback_artvert_image.png").c_str());
		assert( image != NULL && "couldn't load data/fallback_artvert_image.png" );
		fallback_artvert_images.push_back( image );
		// optional, fallback_artvert_image_[0..9].png
		for ( int i=0; i<10; i++ )
		{
			char buf[256];
			sprintf(buf, "fallback_artvert_image_%i.png", i );
			image = avLoadImage( ofToDataPath( buf ).c_str() );
			if ( image != NULL )
				fallback_artvert_images.push_back( image );
		}
	}
	
	which_fallback_image = -1;
}

Artvert::~Artvert()
{
	if ( artvert_image )
		cvReleaseImage( &artvert_image );
	if ( avi_capture )
		delete avi_capture;
	if ( avi_image )
		cvReleaseImage( &avi_image );
}

IplImage* Artvert::getArtvertImage()
{
	if ( artvert_is_movie )
	{
		
		if ( avi_capture == NULL )
		{
			avi_capture = new ofVideoPlayer();
			if ( avi_capture->loadMovie( artvert_movie_file.c_str() ) )
			{
				avi_capture->setLoopState( OF_LOOP_NORMAL );
				avi_capture->play();
				avi_capture->update();
				avi_storage_initialised = false;
			}
			else
				ofLog( OF_LOG_ERROR, "failed to load movie '%s'", artvert_movie_file.c_str() );

		}	
		
		if ( avi_capture->width == 0 )
		{
			if ( which_fallback_image == -1 )
				which_fallback_image = ofRandom( 0, 0.9999f * fallback_artvert_images.size() );
			return fallback_artvert_images[which_fallback_image];
		}

		// to make sure
		avi_capture->play();

		// get the next frame
		IplImage* avi_frame = avGetFrame( avi_capture );
		
		if ( avi_image == 0 )
			avi_image = cvCreateImage( cvGetSize(avi_frame), avi_frame->depth, avi_frame->nChannels );
		cvCopy( avi_frame, avi_image );
		avi_image->origin = avi_frame->origin;
		GLenum format = IsBGR(avi_image->channelSeq) ? GL_BGR_EXT : GL_RGBA;
		
		if (!avi_storage_initialised)
		{
			glGenTextures(1, &imageID);
			glBindTexture(GL_TEXTURE_2D, imageID);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			avi_storage_initialised=true;
		}
		return avi_image;
	}
	else
	{
		if ( !artvert_image )
		{
			printf("loading artvert image '%s'\n", artvert_image_file.c_str() );
			artvert_image = avLoadImage( artvert_image_file.c_str() );
		}
		if ( !artvert_image )
		{
			fprintf(stderr, "couldn't load artvert image '%s'\n", artvert_image_file.c_str() );
			if ( which_fallback_image == -1 )
				which_fallback_image = ofRandom( 0, 0.9999f * fallback_artvert_images.size() );
			artvert_image = fallback_artvert_images[which_fallback_image];
		}
		return artvert_image;
	}
}

void Artvert::stopMovie()
{
	if ( artvert_is_movie && avi_capture != NULL && avi_capture->width != 0 )
		avi_capture->stop();
}

void Artvert::setVolume( float volume )
{
	if ( artvert_is_movie && avi_capture != NULL && avi_capture->width != 0 )
	{
		int passed_volume = volume*255;
		printf("setting volume to %f -> %i\n", volume, passed_volume );
		avi_capture->setVolume( passed_volume );
	}
}

string Artvert::getDescription()
{
	return advert_name + " '" + title + "' [" + artist + "]";
}

void Artvert::loadArtvertsFromXml( ofxXmlSettings& data, vector<Artvert>& results )
{
	Artvert a;
	a.setModelFile( data.getValue( "model_filename", "models/default.bmp" ) );
	// can use either 'advert' or 'name' for the name of this model/advert
	if ( data.getNumTags("advert") != 0 )
		a.advert_name = data.getValue( "advert", "unknown advert" );
	else 
		a.advert_name = data.getValue( "name", "unknown advert" );
	int num_artverts = data.getNumTags( "artvert" );
	printf("   -ml: got advert, model file '%s', advert '%s', %i artverts\n", a.getModelFile().c_str(), a.getAdvertName().c_str(), num_artverts );
	for ( int j=0; j<num_artverts; j++ )
	{
		data.pushTag("artvert", j );
		// can use either 'name' or 'title' here
		if ( data.getNumTags( "name" ) != 0 )
			a.title = data.getValue( "name", "untitled" );
		else
			a.title = data.getValue( "title", "untitled" );
		a.artist = data.getValue( "artist", "unknown artist" );
		if ( data.getNumTags("movie_filename") != 0 )
		{
			// load a movie
			a.artvert_is_movie = true;
			a.setArtvertMovieFile( data.getValue("movie_filename", "artverts/artvertmovie1.mp4" ) );
		}
		else
		{
			// load an image
			a.setArtvertImageFile( data.getValue( "image_filename", "artverts/artvert1.png" ) );
		}
		printf("     %i: %s:%s:%s\n", j, a.title.c_str(), a.artist.c_str(),
			   a.artvert_is_movie?(a.getArtvertMovieFile()+"( movie)").c_str() : a.getArtvertImageFile().c_str() );
		
		results.push_back( a );
		data.popTag();
	}
}	


void Artvert::saveArtvertToXml( ofxXmlSettings& data, Artvert& a )
{
	data.addValue( "model_filename", fromOfDataPath( a.model_file ) );
	// can use either 'advert' or 'name' for the name of this model/advert
	data.addValue( "name", a.advert_name );
	int index = data.addTag( "artvert" );
	data.pushTag( "artvert", index );
	data.addValue( "title", a.title );
	data.addValue( "artist", a.artist );
	if ( a.artvert_is_movie )
	{
		data.addValue( "movie_filename", fromOfDataPath( a.getArtvertMovieFile() ) );
	}
	else
	{
		data.addValue( "image_filename", fromOfDataPath( a.getArtvertImageFile() ) );
	}
	data.popTag();
}	



void ArtvertDrawer::draw( float x, float y, float w, float h )
{
	if ( artvert== NULL )
		return;

	IplImage* artvert_image = artvert->getArtvertImage();
	if ( artvert->which_fallback_image != -1 )
		local_image.clear();
	else
		toOfImage( artvert_image, local_image );
	
	local_image.draw( x, y, w, h );
}


