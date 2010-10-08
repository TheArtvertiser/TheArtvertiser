/*
 *  ofxBarrier.cpp
 *  artvertiser
 *
 *  Created by damian on 05/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "ofxBarrier.h"

ofxBarrier::ofxBarrier( int _count )
: turnstile( 0 ), turnstile2( 0 )
{
	n = _count;
	count = 0;
}


void ofxBarrier::phase1()
{
	mutex.lock();
	count++;
	if ( count == n )
		turnstile.signal(n);
	mutex.unlock();
	turnstile.wait();
}

void ofxBarrier::phase2()
{
	mutex.lock();
	count--;
	if ( count == 0 )
		turnstile2.signal(n);
	mutex.unlock();
	turnstile2.wait();
}

void ofxBarrier::wait()
{
	phase1();
	phase2();
}

