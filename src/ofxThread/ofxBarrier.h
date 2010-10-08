/*
 *  Barrier.h
 *  artvertiser
 *
 *  Created by damian on 05/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */


#pragma once

#include <pthread.h>

#include "ofxMutex.h"
#include "ofxSemaphore.h"

/** ofxBarrier
 
 a barrier, after the one given in The Little Book of Semaphores (Downey 2005)
 */

class ofxBarrier
{
public:
	
	/// setup the barrier to wait for count threads before releasing
	ofxBarrier( int count );
	
	/// the current thread should block here until all count threads have reached the barrier
	void wait();
	
	
	
private:

	void phase1();
	void phase2();
	
	int n,count;
	ofxMutex mutex;
	ofxSemaphore turnstile, turnstile2;
	
};

