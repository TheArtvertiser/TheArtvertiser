/*
 Copyright 2008, 2009, 2010 Damian Stewart <damian@frey.co.nz>.
 Distributed under the terms of the GNU General Public License v3.

 This file is part of The Artvertiser.

 The Artvertiser is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The Artvertiser is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with The Artvertiser.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ofMain.h"
#include "FThread.h"
#ifdef TARGET_WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#include <sys/errno.h>
#endif
#include <signal.h>


void FThread::StartThread( int thread_priority )
{
	if ( thread_running ) {
	    printf("FThread::Start(): FThread %x already running\n", this );
        return;
	}
	thread_should_stop = false;

    #ifndef TARGET_WIN32
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
    #endif

    // launch
    #ifdef TARGET_WIN32
    the_thread = (HANDLE)_beginthreadex(NULL, 0, run_function,  (void *)this, 0, NULL);
    #else
	int result = pthread_create( &the_thread, &thread_attr, run_function, this );
    if ( result != 0 )
    {
        fprintf(stderr, "FThread %x: pthread_create failed with error %i\n", this, result );
    }
    // priority
    if ( thread_priority > 0 )
    {
        printf("FThread %x attempting to set thread priority to %i\n", this ,thread_priority );
        struct sched_param param;
        param.sched_priority = thread_priority;
        int res = pthread_setschedparam( the_thread, SCHED_RR, &param );
        if ( res != 0 )
        {
            fprintf(stderr,"pthread_setschedparam failed: %s\n",
                   (res == ENOSYS) ? "ENOSYS" :
                   (res == EINVAL) ? "EINVAL" :
                   (res == ENOTSUP) ? "ENOTSUP" :
                   (res == EPERM) ? "EPERM" :
                   (res == ESRCH) ? "ESRCH" :
                   "???"
                   );
        }
    }

    pthread_attr_destroy( &thread_attr );
    #endif

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
	#ifdef TARGET_WIN32
	WaitForSingleObject(the_thread, INFINITE);
	#else
	void * ret;
	pthread_join( the_thread, &ret );
	#endif
	thread_running = false;
}

/*
long FThread::GetCurrentThreadId()
{
	pthread_t id = pthread_self();
	return (long)id.p;
}*/


