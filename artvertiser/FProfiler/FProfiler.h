/*
 Copyright 2008, 2009, 2010 Damian Stewart <damian@frey.co.nz>.

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

#ifndef __PROFILER_H
#define __PROFILER_H


/** FProfiler

    A thread-safe profiler. On call to Display(), displays result by thread.

    To use, #include "FProfiler.h", and globally #define PROFILE. Then, use
    the following macros to profile parts of code:

    * PROFILE_SECTION_PUSH( label ) and PROFILE_SECTION_POP()

        Profile code between these calls, storing results in a tree.
        For example:

            .. unprofiled preprocessing
            PROFILE_SECTION_PUSH( "partial section" );
            .. code to be profiled
            PROFILE_SECTION_POP();
            .. unprofiled postprocessing

        Or for nested output:

            PROFILE_SECTION_PUSH( "two-step process" );
                .. preprocessing code
                PROFILE_SECTION_PUSH( "step 1" );
                .. step 1 code
                PROFILE_SECTION_POP();
                PROFILE_SECTION_PUSH( "step 2" );
                .. step 2 code
                PROFILE_SECTION_POP();
            PROFILE_SECTION_POP();

        will yield the output
            - two-step process      <total time for steps 1 + 2 + preprocessing> ...
            - - step 1              <time for step 1> ...
            - - step 2              <time for step 2> ...

        NOTE: all labels at a given level in the tree must be unique.

    * PROFILE_THIS_FUNCTION()

        Wraps the current function in a pair of
        PROFILE_SECTION_PUSH( function_name ) and PROFILE_SECTION_POP calls.

    * PROFILE_THIS_BLOCK( label )

        Wraps the current block (the code between the current level { and })
        in a pair of PROFILE_SECTION_PUSH( label ) and PROFILE_SECTION_POP
        calls.

        eg:

            if ( test_condition )
            {
                PROFILE_THIS_BLOCK( "test passed" );
                // code to run on test condition
            }


    To display profile results, call FProfiler::Display();

@author Damian

*/


/// macros
#ifdef PROFILE
#define PROFILE_SECTION_PUSH( label ) FProfiler::SectionPush( label );
#define PROFILE_SECTION_POP() FProfiler::SectionPop();
#define PROFILE_THIS_FUNCTION() volatile FFunctionProfiler __function_profiler_object__( __FUNCTION__ );
#define PROFILE_THIS_BLOCK( label ) volatile FFunctionProfiler __section_profiler_object__##__LINE__( label );
#warning Profiling with FProfiler enabled
#else
#define PROFILE_SECTION_PUSH( label ) ;
#define PROFILE_SECTION_POP() ;
#define PROFILE_THIS_FUNCTION() ;
#define PROFILE_THIS_BLOCK( label );
//#warning Profiling with FProfiler disabled
#endif

#include "ofMain.h"

#include "ofxMutex.h"
#include "FTime.h"
#include "FThread.h"
#include <map>
#include <string>
#include <vector>


class FProfileSection;

class FProfileContext
{
public:
	FThreadContext thread_context;
	FProfileSection* toplevel;
	FProfileSection* current;

    FProfileContext() { toplevel = NULL; }
    ~FProfileContext();
};


class FProfiler
{
public:
    /// clear the database and restart profiling
    static void Clear();

	/// start a section
	static void SectionPush( const std::string& name = "unlabelled section" );
	/// end a section
	static void SectionPop();

	/// return a pointer to the context for the current thread
	static FProfileContext* GetContext();

	/// show profiles recorded. SORT_BY defines sort order.
	typedef enum _SORT_BY { SORT_EXECUTION, SORT_TIME } SORT_BY;
	static void Display( SORT_BY sort = SORT_TIME );

private:

    // for efficiency: avoid re-allocating
    static FTime end_time;


	typedef std::vector<FProfileContext*> FProfileContexts;
	static FProfileContexts contexts;

	static ofxMutex lock;
};

// one section
class FProfileSection {
public:
	FProfileSection();
	~FProfileSection();

	void Display( const std::string& prefix, FProfiler::SORT_BY sort_by );

	int call_count;
	double avg_time;
	int exec_order_id;
	static int EXEC_ORDER_ID;

	FTime timer;
//	LARGE_INTEGER start_time;

	FProfileSection* parent;
	std::string name;

	// map of sections
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




/** FFunctionProfiler

  convenience class. designed to be used as a volatile instance, within a function/block.

*/

class FFunctionProfiler
{
public:
	FFunctionProfiler( const char* function_name )
	{	FProfiler::SectionPush(function_name);	}
	~FFunctionProfiler()
	{	FProfiler::SectionPop(); }
};



#endif
