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
	int result = pthread_create( &the_thread, NULL, run_function, this );
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
	int err = pthread_kill( the_thread, SIGTERM );
	if ( err != 0 && err != ESRCH )
	{
	    printf("FThread::Stop(): error %i killing FThread %x\n", err, this );
	}
	thread_running = false;
}

/*
long FThread::GetCurrentThreadId()
{
	pthread_t id = pthread_self();
	return (long)id.p;
}*/
