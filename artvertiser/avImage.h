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

/// strip off the base path prepended by ofToDataPath, returning paths relative
/// to the data/ directory
string fromOfDataPath( string path );

/// convert the given path to an absolute (/) path by prepending cwd
string toAbsolutePath( string path );
/// strip off cwd from the given path
string fromAbsolutePath( string path );


int avSaveImage( const char* path, CvArr* image );

IplImage* avLoadImage( const char* path, int is_color=-1 );

/// return the current frame. YOU MUST NOT FREE THE RETURNED IMAGE.
IplImage* avGetFrame( ofBaseVideo* video_source );









