/*
 *  FTime.h
 *  F
 *
 *  Created by damian on 21/03/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _FTIME_H
#define _FTIME_H

/** FTime

 implements a high resolution timer

 can store either a high-precision relative time or a high-precision absolute time

*/

#ifdef OSX
#include <mach/mach_time.h>
#else
#include <time.h>
#endif

class FTime
{
public:
	FTime() {
		#ifdef OSX
		time = 0;
		#else
		time.tv_sec = 0; time.tv_nsec = 0;
		#endif
		 last_update_time = 0.1f; };
	~FTime() { };

	// set us to the current time
	void SetNow();

	// update our time to the current time, returning
	// delta time in seconds as a float
	double Update();

	// get the last delta time returned by Update()
	double GetLastUpdateTime() { return last_update_time; }

	// return seconds
	double ToMillis();

	// calculate difference
	FTime operator-( const FTime& other ) const;
	FTime& operator -= (const FTime& other );

private:
	#ifdef OSX
	uint64_t time;
	#else
	timespec time;
	#endif
	double last_update_time;

};


#endif
