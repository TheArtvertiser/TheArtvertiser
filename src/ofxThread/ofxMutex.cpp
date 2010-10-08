/*
 *  ofxMutex.cpp
 *  artvertiser
 *
 *  Created by damian on 05/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "ofxMutex.h"
#include <errno.h>
#include <assert.h>

ofxMutex::ofxMutex()
{
#ifdef TARGET_WIN32 
	InitializeCriticalSection(&critSec); 
#else 
	int res = pthread_mutex_init(&myMutex, NULL); 
	if ( res == -1 ) 
	{
		printf("couldn't pthread_mutex_init: %i %s\n", errno, strerror(errno) );
		assert( false && "couldn't pthread_mutex_init\n");
	}
#endif 
} 

ofxMutex::~ofxMutex()
{
#ifdef TARGET_WIN32 
	DeleteCriticalSection(&critSec);
#else
	pthread_mutex_destroy(&myMutex); 
#endif 
} 
