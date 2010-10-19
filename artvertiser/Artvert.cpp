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
	advert = "unknown advert";
	name = "unnamed artvert";
	avi_capture = NULL;
	avi_image = NULL;
	avi_play_init = false;
	
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
		
		if ( avi_capture==NULL )
		{
			ofVideoPlayer * avi_cap = new ofVideoPlayer();
			avi_cap->loadMovie( artvert_movie_file.c_str() );
			avi_cap->setLoopState( OF_LOOP_NORMAL );
			avi_cap->play();
			avi_cap->update();
			avi_capture = avi_cap;
			avi_play_init = false;
		}	
		
		// get the next frame
		IplImage* avi_frame = avGetFrame( avi_capture );
		
		if ( avi_frame == 0 )
		{
			printf("failed to load movie '%s'\n", artvert_movie_file.c_str() );
			if ( which_fallback_image == -1 )
				which_fallback_image = ofRandom( 0, 0.9999f * fallback_artvert_images.size() );
			return fallback_artvert_images[which_fallback_image];
		}
		if ( avi_image == 0 )
			avi_image = cvCreateImage( cvGetSize(avi_frame), avi_frame->depth, avi_frame->nChannels );
		cvCopy( avi_frame, avi_image );
		avi_image->origin = avi_frame->origin;
		GLenum format = IsBGR(avi_image->channelSeq) ? GL_BGR_EXT : GL_RGBA;
		
		if (!avi_play_init)
		{
			glGenTextures(1, &imageID);
			glBindTexture(GL_TEXTURE_2D, imageID);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			avi_play_init=true;
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

