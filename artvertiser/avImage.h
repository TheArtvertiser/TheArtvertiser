/*
 *  avImage.h
 *  artvertiser
 *
 *  Created by damian on 05/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#pragma once

// handle image stuff internally

#include <cv.h>
#include "ofMain.h"

#ifndef IsRGB
#define IsRGB(s) ((s[0] == 'R') && (s[1] == 'G') && (s[2] == 'B'))
#endif
#ifndef IsBGR
#define IsBGR(s) ((s[0] == 'B') && (s[1] == 'G') && (s[2] == 'R'))
#endif

/// strip off the base absolute or ofDataPath part
string fromOfDataOrAbsolutePath( string path );

/// strip off the base path prepended by ofToDataPath, returning paths relative
/// to the data/ directory
string fromOfDataPath( string path );

/// convert the given path to an absolute (/) path by prepending cwd
string toAbsolutePath( string path );
/// strip off cwd from the given path
string fromAbsolutePath( string path );


/// save the given image to the given absolute path
int avSaveImage( const char* path, CvArr* image );

/// load the given image from the given absolute path. see cvLoadImage for is_color argument.
IplImage* avLoadImage( const char* path, int is_color=-1 );

/// return the current frame. YOU MUST NOT FREE THE RETURNED IMAGE.
IplImage* avGetFrame( ofBaseVideo* video_source );



/// copy the given IplImage* into the given ofImage
void toOfImage( IplImage* source, ofImage& destination );






