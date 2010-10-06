/*
 *  avImage.cpp
 *  artvertiser
 *
 *  Created by damian on 05/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "avImage.h"

#include <map>

#include "ofxOpenCv.h"

typedef map< ofBaseVideo*, ofxCvColorImage > WorkingFrames;

class WorkingFramesDatabase
{
public:

	ofxCvColorImage& operator[] ( ofBaseVideo* source )
	{
		return working[ source ];
	}
	WorkingFrames::iterator find( ofBaseVideo* source )
	{
		return working.find( source );
	}
	WorkingFrames::iterator end() 
	{
		return working.end();
	}
	
	void erase( ofBaseVideo* source )
	{
		working.erase( source );
	}
	
	WorkingFrames working;
	
};

WorkingFramesDatabase working_frames;

IplImage* avGetFrame( ofBaseVideo* video_source )
{
	// get a frame from the video source
	
	bool allocate = false;
	ofxCvColorImage* working_frame = NULL;
	
	// do we have one?
	if ( working_frames.find( video_source )== working_frames.end() )
	{
		allocate = true;
	}
	else
	{
		// is the one we have the right size?
		working_frame = &working_frames[video_source];
		CvSize size = cvGetSize( working_frame );
		if ( size.width != video_source->getWidth() ||
			size.height != video_source->getHeight() )
		{
			working_frames.erase( video_source );
			working_frame = NULL;
			allocate = true;
		}
	}

	// allocate
	if ( allocate )
	{
		// 24 bit RGB
		working_frame = &working_frames[video_source];
		working_frame->allocate( video_source->getWidth(), video_source->getHeight() );
	}
	
	// now copy
	working_frame->setFromPixels( video_source->getPixels(), video_source->getWidth(), video_source->getHeight() );
	
	return working_frame->getCvImage();
}



int avSaveImage( const char* path, CvArr* image )
{
	return 0;
}


IplImage* avLoadImage( const char* path, int is_color )
{
	return NULL;
}

