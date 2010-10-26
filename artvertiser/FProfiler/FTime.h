/*
 Copyright 2007, 2008, 2009, 2010 Damian Stewart <damian@frey.co.nz>.
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

#ifndef _FTIME_H
#define _FTIME_H

/** FTime

 implements a high resolution timer

 can store either a high-precision relative time or a high-precision absolute time

*/

#include "ofMain.h"

#ifdef TARGET_OSX
#include <mach/mach_time.h>
#elif defined TARGET_WIN32
#include <windows.h>
#else
#include <time.h>
#endif
#include <stdio.h>

#include <assert.h>

class FTime
{
public:
	FTime() {
		#if defined TARGET_OSX
		time = 0;
		#elif defined TARGET_WIN32
		time.QuadPart = 0;
		#else
		time.tv_sec = 0; time.tv_nsec = 0;
		#endif
		 last_update_time = 0.1f; };
	~FTime() { };
	FTime( const FTime& other ) { Copy(other); }

	// set us to the current time
	void SetNow()
	{
        #if defined TARGET_OSX
        time = mach_absolute_time();
        #elif defined TARGET_WIN32
        if ( !is_initialised )

        QueryPerformanceCounter(&time);
        #else
        // clock_gettime(CLOCK_MONOTONIC, &ts); // Works on FreeBSD
        clock_gettime(CLOCK_REALTIME, &time); // Works on Linux
        #endif
    }

	// set us to the given time in seconds
	void SetSeconds( double seconds );

	// update our time to the current time, returning
	// delta time in seconds as a float
	double Update();

	// get the last delta time returned by Update()
	double GetLastUpdateTime() const { return last_update_time; }

	// return milliseconds
	double ToMillis() const;
	// return seconds
	double ToSeconds() const { return ToMillis() / 1000.0; }


	// calculate difference
	FTime operator-( const FTime& other ) const
	{
        // copy self
        FTime t = *this;
        // invoke operator -=
        t -= other;
        return t;
	}
	FTime& operator -= (const FTime& other )
	{
    	#if defined TARGET_OSX
        time -= other.time;
        #elif defined TARGET_WIN32
        time.QuadPart -= other.time.QuadPart;
        #else
        //assert( false && "broken code, please fix ");
        if ( time.tv_nsec < other.time.tv_nsec )
        {
            // will underflow

            //printf("underflow: %10li:%10li - %10li:%10li = ", time.tv_sec, time.tv_nsec, other.time.tv_sec, other.time.tv_nsec );
            time.tv_sec -= other.time.tv_sec + 1;
            time.tv_nsec = 1e9 - (other.time.tv_nsec-time.tv_nsec);
            //printf(" %10li:%10li\n", time.tv_sec, time.tv_nsec );
        }
        else
        {
            time.tv_sec -= other.time.tv_sec;
            time.tv_nsec -= other.time.tv_nsec;
        }
        #endif
        return *this;
	}

	// assignment
	FTime& operator= (const FTime& other ) { Copy(other); return *this; }

	// equality
	bool operator== (const FTime& other ) const {
	    #if defined TARGET_OSX
		return ( time == other.time );
		#elif defined TARGET_WIN32
		return ( time.QuadPart == other.time.QuadPart );
	    #else
	    return time.tv_sec == other.time.tv_sec && time.tv_nsec == other.time.tv_nsec;
	    #endif
	     }

	// compare
	bool operator< (const FTime& other) const {
        #if defined TARGET_OSX
		return ( time < other.time );
		#elif defined TARGET_WIN32
		return ( time.QuadPart < other.time.QuadPart );
        #else
        /*bool res =*/return (time.tv_sec < other.time.tv_sec) ||
            (time.tv_sec == other.time.tv_sec && time.tv_nsec < other.time.tv_nsec );
            /*
        printf("%f < %f: %s\n", ToSeconds(), other.ToSeconds(), res?"yes":"no");
        return res;*/
        #endif
        };

private:

    void Copy( const FTime& other )
    {
        if ( this != &other )
        {
            time = other.time;
        }
    }


	#if defined TARGET_OSX
	uint64_t time;
	#elif defined TARGET_WIN32
	LARGE_INTEGER time;
	#else
	timespec time;
	#endif
	double last_update_time;

    #ifdef TARGET_WIN32
    static bool is_initialised;
    static double performance_frequency;
    #endif

};

//inline bool operator< (const FTime& a, const FTime& b) { return a.operator<(b); }

#endif
