/*
 *  FThread.h
 *  F
 *
 *  Created by damian on 25/5/08.
 *  Copyright 2008 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "FThread.h"
#include <pthread.h>
#include <sys/errno.h>
#include <signal.h>

void FThread::StartThread()
{
	if ( thread_running ) {
	    printf("FThread::Start(): FThread %x already running\n", this );
        return;
	}
	thread_should_stop = false;

    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
    // launch
	int result = pthread_create( &the_thread, &thread_attr, run_function, this );
    pthread_attr_destroy( &thread_attr );
	assert( result == 0 );
	pthread_detach( the_thread );
	thread_running = true;
}

void FThread::StopThread()
{
	if ( !thread_running ) {
	    printf("FThread::Stop(): FThread %x not running\n", this );
        return;
	}
	printf("stopping FThread %x\n", this );
	thread_should_stop = true;
	void * ret;
	pthread_join( the_thread, &ret );
	thread_running = false;
}

/*
long FThread::GetCurrentThreadId()
{
	pthread_t id = pthread_self();
	return (long)id.p;
}*/
