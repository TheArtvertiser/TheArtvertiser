/*
 *  avImage.h
 *  artvertiser
 *
 *  Created by damian on 05/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

// handle image stuff internally

#include <cv.h>
#include "ofMain.h"

string fromOfDataPath( string path );


int avSaveImage( const char* path, CvArr* image );

IplImage* avLoadImage( const char* path, int is_color=-1 );

/// return the current frame. DO NOT FREE THE RETURNED IMAGE.
IplImage* avGetFrame( ofBaseVideo* video_source );









