/*
 *  ofxSemaphore.h
 *  artvertiser
 *
 *  Created by damian on 05/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#pragma once

#include "ofConstants.h"

#ifdef TARGET_WIN32
#include <process.h>
#elif defined TARGET_OSX
#include <mach/semaphore.h>
#include <mach/task.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif

class ofxSemaphore
{
public:
	ofxSemaphore( unsigned int start_value );
	~ofxSemaphore();
	
	/// wait on the semaphore.
	void wait();
	/// increment the value of the semaphore. times is the number of times to increment.
	void signal( int times=1 );
	/// return the value of the semaphore
	int getValue();

private:
#ifdef TARGET_WIN32
	HANDLE semaphore;
#elif defined TARGET_OSX
	semaphore_t semaphore;
#else
	sem_t semaphore;
#endif

};
