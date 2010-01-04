/*
 *  FThread.h
 *  F
 *
 *  Created by damian on 25/5/08.
 *  Copyright 2008 frey damian@frey.co.nz. All rights reserved.
 *
 */

#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>
#include <assert.h>
#include <stdio.h>

/** base class for threads */

class FThread
{
public:
    FThread( ) { thread_running = false; }
    virtual ~FThread() { if ( thread_running ) StopThread(); }

    /// start running the ThreadedFunction
    void StartThread();
    /// stop running ThreadedFunction explicitly. not recommended..
    void StopThread();

protected:

    /// override this to do what you want on this object
    virtual void ThreadedFunction() { printf("FThread::ThreadedFunction running.. override me!\n"); }


    /// called internally
    static void * run_function(void * objPtr){
        FThread* the_fthread = (FThread*)objPtr;
        the_fthread->ThreadedFunction();
        the_fthread->thread_running = false;

        pthread_exit(0);
        return 0;
    }

    pthread_t the_thread;
    bool thread_running;

};

class FThreadContext
{
public:
	FThreadContext() { Set(); }

	/// set to the current thread context
	void Set() { context = pthread_self(); }

	/// equal?
	bool operator == ( const FThreadContext& other ) const { return pthread_equal( context, other.context ); }

private:
	pthread_t context;
};

#endif
