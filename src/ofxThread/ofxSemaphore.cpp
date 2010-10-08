/*
 *  ofxSemaphore.cpp
 *  artvertiser
 *
 *  Created by damian on 05/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "ofxSemaphore.h"
#include <errno.h>
#include <assert.h>

ofxSemaphore::ofxSemaphore( unsigned int start_value )
{
#ifdef TARGET_WIN32
	static const int MAX_VALUE = 256; // ???
	semaphore = CreateSemaphore( NULL, start_value, MAX_VALUE, NULL );
#elif defined TARGET_OSX
	semaphore_create( mach_task_self(), &semaphore, SYNC_POLICY_FIFO, start_value );
#else
	int res = sem_init( &semaphore, /*shared*/0, start_value );
	if ( res == -1 )
	{
		printf("couldn't create semaphore: %i: %s\n", errno, strerror( errno ) );
		assert( false && "couldn't create semaphore" );
	}
#endif

}

ofxSemaphore::~ofxSemaphore()
{
#ifdef TARGET_WIN32
	CloseHandle( semaphore );
#elif defined TARGET_OSX
	semaphore_destroy( mach_task_self(), semaphore );
#else
	sem_destroy( &semaphore );
#endif
}

int ofxSemaphore::getValue()
{
#if defined TARGET_WIN32 || defined TARGET_OSX
	assert(false&& "can't do this on win32 or OSX");
#else
	int value;
	sem_getvalue( &semaphore, &value );
	return value;
#endif
}

void ofxSemaphore::wait()
{
#ifdef TARGET_WIN32
	WaitForSingleObject( semaphore, 0 );
#elif defined TARGET_OSX
	semaphore_wait( semaphore );
#else
	sem_wait( &semaphore );
#endif
}

void ofxSemaphore::signal( int times )
{
#ifdef TARGET_WIN32
	ReleaseSemaphore( semaphore, times, NULL );
#elif defined TARGET_OSX
	for ( int i=0; i<times; i++ )
		semaphore_signal( semaphore );
#else
	for ( int i=0; i<times; i++ )
		sem_post( &semaphore );
#endif
}
