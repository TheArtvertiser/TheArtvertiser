/*
 *  ofxMutex.h
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
#else
	#include <pthread.h>
#endif


/**
 ofxMutex
 
 a basic mutex
 
 */

class ofxMutex
{
public:
	/// initially unlocked
	ofxMutex();
	~ofxMutex();
	
	/// lock this mutex. if already locked by another thread, block until available and then lock.
	void lock() { 
		// inlined for efficiency
		#ifdef TARGET_WIN32
			EnterCriticalSection(&critSec);
		#else
			pthread_mutex_lock(&myMutex);
		#endif
	}
	/// attempt to lock this mutex. return true if locking succeeded, or false if the mutex is locked elsewhere.
	bool tryLock()
	{
		// inlined for efficiency
		int result;
		#ifdef TARGET_WIN32
			result = TryEnterCriticalSection(&critSec);
			return (result==1);
		#else
			result = pthread_mutex_trylock(&myMutex);
			return (result==0);
		#endif
		
	}
		
	/// unlock this mutex.
	void unlock()
	{
		// inlined for efficiency
		#ifdef TARGET_WIN32
			LeaveCriticalSection(&critSec);
		#else
			pthread_mutex_unlock(&myMutex);
		#endif
	}

private:
	
#ifdef TARGET_WIN32
	CRITICAL_SECTION  critSec;  	//same as a mutex
#else
	pthread_mutex_t  myMutex;
#endif

};

