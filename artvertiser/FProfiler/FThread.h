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

    /// start running the ThreadedFunction. thread_priority only takes effect if running as root.
    void StartThread( int thread_priority = 0 ) ;
    /// stop running ThreadedFunction
    void StopThread();

protected:

    /// override this to do what you want on this object
    /// called repeatedly until the thread should die
    virtual void ThreadedFunction() { printf("FThread::ThreadedFunction running.. override me!\n"); }


    /// called internally
    static void * run_function(void * objPtr){
        FThread* the_fthread = (FThread*)objPtr;

        while( !the_fthread->thread_should_stop )
            the_fthread->ThreadedFunction();

        the_fthread->thread_running = false;

        pthread_exit(0);
        return 0;
    }

    pthread_t the_thread;
    bool thread_running;
    bool thread_should_stop;

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
