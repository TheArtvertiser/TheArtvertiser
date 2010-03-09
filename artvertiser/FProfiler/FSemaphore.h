/*
 *  FSemaphore.h
 *  F
 *
 *  Created by damian on 25/5/08.
 *  Copyright 2008 frey damian@frey.co.nz. All rights reserved.
 *
 */

#ifndef _FSEMAPHORE_H
#define _FSEMAPHORE_H

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/errno.h>
#include <assert.h>
#ifndef __APPLE__
#include <malloc.h>
#endif

#ifdef __APPLE__
//#warning compiling under OSX
#include <boost/thread/barrier.hpp>
#endif

class FBarrier
{
public:
    /// count is the number of threads to wait for.
    FBarrier( int count )
    #ifdef __APPLE__
    : barrier( count ) {};
    #else
    {
        int res = pthread_barrier_init( &barrier, 0, count );
        assert(res==0 && "error creating barrier");
    }
    #endif
    ~FBarrier()
    {
        #ifdef __APPLE__
        #else
        int res = pthread_barrier_destroy( &barrier );
        if ( res==EBUSY )
            assert(false && "cannot destroy barrier when it is in use" );
        #endif
    }


    /// wait for everyone to arrive at the barrier. return once they have. returns true if wait returns
    /// the PTHREAD_BARRIER_SERIAL_THREAD return value.
    bool Wait() {
        #ifdef __APPLE__
        return barrier.wait();
        #else
        int ret = pthread_barrier_wait( &barrier );
        return ret == PTHREAD_BARRIER_SERIAL_THREAD;
        #endif
        }

private:

#ifdef __APPLE__
	boost::barrier barrier;
#else
    pthread_barrier_t barrier;
#endif

};

class FSemaphore
{
public:
	FSemaphore( int init = 1,bool _debug = false) { debug = _debug; 
#ifdef __APPLE__
		char buf[64];
		sprintf( buf, "sem%lx", (unsigned long)this );
		sem = sem_open( buf, O_CREAT, 0666, init );
		if( sem == SEM_FAILED )
		{
			fprintf(stderr, "semaphore creation failed: errno is %i\n", errno );
			assert(false);
		}
#else
		sem = (sem_t*)malloc( sizeof( sem_t ) );
		int res = sem_init( sem, 0, init ); 
		if ( res == -1 )
		{
			fprintf(stderr, "semaphore creation falide: erron is %i\n", errno );
			assert(false);
		}
#endif	
	}
	~FSemaphore() { 
#ifdef __APPLE__
		sem_close( sem );
#else
		sem_destroy( sem ); 
		free( sem );
#endif
	}

	/// wait for the semaphore to become available and then grab
	void Wait() { sem_wait( sem ); }

	/// try once; if available, grab and return true, else return false
	bool TryWait() {
		/*#ifdef DEBUG
		int err=sem_trywait( &sem );
		if ( err != 0 )
		Log::Msg( "trywait: error %s\n", err == EAGAIN? "EAGAIN" :
				  (err==EDEADLK?"EDEADLK":(err==EINTR?"EINTR":(err==EINVAL?"EINVAL":"UNKNOWN"))));
		return ( err == 0 );
#else*/
		return 0==sem_trywait( sem );
		//#endif
	}

	/// signal the semaphore that we've finished
	void Signal() { sem_post( sem );  }

private:
	sem_t* sem;
	bool debug;
};


#endif

