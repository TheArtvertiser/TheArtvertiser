/*
 *  ThreadSafeString.h
 *  artvertiser
 *
 *  Created by damian on 23/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#pragma once
#include <string>
using namespace std;
#include "ofxMutex.h"

class ThreadSafeString { 
public:
	void setText( const string& new_text ) { lock(); text=new_text; unlock(); }
	string getText() { string result; lock(); result = text; unlock(); return result; }
	void operator=( const string& new_text ) { setText( new_text ); }
private:
	string text;
	ofxMutex mutex;
	void lock() { mutex.lock(); }
	void unlock() { mutex.unlock(); }
};
