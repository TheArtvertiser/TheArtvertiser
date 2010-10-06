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

int avSaveImage( const char* path, CvArr* image );

IplImage* avLoadImage( const char* path, int is_color=1 );

IplImage* avGetFrame( ofBaseVideo* video_source );









