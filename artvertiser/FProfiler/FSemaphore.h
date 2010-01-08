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

class FBarrier
{
public:
    /// count is the number of threads to wait for.
    FBarrier( int count ) { int res = pthread_barrier_init( &barrier, 0, count ); assert(res==0 && "error creating barrier"); }
    ~FBarrier() { int res = pthread_barrier_destroy( &barrier ); if ( res==EBUSY ) assert(false && "cannot destroy barrier when it is in use" ); }

    /// wait for everyone to arrive at the barrier. return once they have. returns true if wait returns
    /// the PTHREAD_BARRIER_SERIAL_THREAD return value.
    bool Wait() { int ret = pthread_barrier_wait( &barrier ); return ret == PTHREAD_BARRIER_SERIAL_THREAD; }

private:

    pthread_barrier_t barrier;

};

class FSemaphore
{
public:
	FSemaphore( int init = 1,bool _debug = false) { debug = _debug; sem_init( &sem, 0, init ); }
	~FSemaphore() { sem_destroy( &sem ); }

	/// wait for the semaphore to become available and then grab
	void Wait() { if ( debug ) printf("%x about to Wait\n", (unsigned int)&sem ); sem_wait( &sem ); if( debug ) printf("%x Wait\n", (unsigned int)&sem ); }

	/// try once; if available, grab and return true, else return false
	bool TryWait() {
		/*#ifdef DEBUG
		int err=sem_trywait( &sem );
		if ( err != 0 )
		Log::Msg( "trywait: error %s\n", err == EAGAIN? "EAGAIN" :
				  (err==EDEADLK?"EDEADLK":(err==EINTR?"EINTR":(err==EINVAL?"EINVAL":"UNKNOWN"))));
		return ( err == 0 );
#else*/
		if ( debug ) printf( "%x Trywait\n", (unsigned int)&sem );
		return 0==sem_trywait( &sem );
		//#endif
	}

	/// signal the semaphore that we've finished
	void Signal() { if (debug) printf("%x Signal\n", (unsigned int)&sem); sem_post( &sem );  }

private:
	sem_t sem;
	bool debug;
};


#endif
