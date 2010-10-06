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

