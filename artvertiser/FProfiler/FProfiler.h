/*
 *  FProfiler.h
 *  F
 *
 *  Created by damian on 25/5/08.
 *  Copyright 2008 frey damian@frey.co.nz. All rights reserved.
 *
 */

#ifndef __PROFILER_H
#define __PROFILER_H


/** Profiler

A thread-safe profiler.

On call to display, displays result by thread.

#define PROFILE to enable

@author Damian

*/



#ifdef PROFILE
#define PROFILE_SECTION_START( label ) FProfiler::SectionStart( label );
#define PROFILE_SECTION_END() FProfiler::SectionEnd();
#define PROFILE_THIS_FUNCTION() volatile FFunctionProfiler __function_profiler_object__( __FUNCTION__ );
#define PROFILE_THIS_BLOCK( label ) volatile FFunctionProfiler __section_profiler_object__##__LINE__( label );
#else
#define PROFILE_SECTION_START( label ) ;
#define PROFILE_SECTION_END() ;
#define PROFILE_THIS_FUNCTION() ;
#define PROFILE_THIS_BLOCK( label );
#endif


#include <FSemaphore.h>
#include <FTime.h>
#include <FThread.h>
#include <map>
#include <string>
#include <vector>

// one section
class FProfileSection {
public:
	FProfileSection();
	void Display( const std::string& prefix );
	
	int call_count;
	double avg_time;
	
	FTime start_time;
//	LARGE_INTEGER start_time;
	
	FProfileSection* parent;
	
	// vector of sections
	typedef std::map<const std::string, FProfileSection* > FProfileSections;
	
	// don't try this at home
	/*struct less_than_comparator : public std::binary_function<const ProfileSection*,const ProfileSection*,bool>
	{
		result_type operator() ( first_argument_type a, second_argument_type b )
	{
			return ( a->avg_time * a->call_count < b->avg_time*b->call_count );
	}
	};*/
	FProfileSections children;
	
	
};


typedef struct _FProfileContext
{
	FThreadContext thread_context;
	FProfileSection* toplevel;
	FProfileSection* current;
} FProfileContext;


class FProfiler
{
public:
	/// start a section
	static void SectionStart( const std::string& name );
	/// end a section
	static void SectionEnd();
	
	/// return a pointer to the context for the current thread
	static FProfileContext* GetContext();
	
	/// show profiles recorded
	typedef enum _SORT_BY { SORT_COUNT, SORT_TIME } SORT_BY;
	static void Display( SORT_BY sort = SORT_TIME );
	
private:
		
	typedef std::vector<FProfileContext*> FProfileContexts;
	static FProfileContexts contexts;
	
	static FSemaphore lock;
};


class FFunctionProfiler
{
public:
	FFunctionProfiler( const char* function_name )
	{	FProfiler::SectionStart(function_name);	}
	~FFunctionProfiler()
	{	FProfiler::SectionEnd(); }
};



#endif
