/*
 Copyright 2010 Damian Stewart <damian@frey.co.nz>
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
 

#pragma once

#include <pthread.h>
#include <stdio.h>
#include "FProfiler/FThread.h"
#include "FProfiler/FSemaphore.h"
#include "FProfiler/FTime.h"
#include <cv.h>
#include <highgui.h>
#include <map>

class MultiThreadCapture;

using namespace std;

class MultiThreadCaptureManager
{
public:
    MultiThreadCaptureManager() {};
    ~MultiThreadCaptureManager() { instance = NULL; }
    static MultiThreadCaptureManager* getInstance() { if ( instance == NULL ) instance = new MultiThreadCaptureManager(); return instance; }

    /// add the given MultiThreadCapture* for the given cam.
    void addCaptureForCam( CvCapture* cam, MultiThreadCapture* cap );
    /// fetch the MultiThreadCapture for the given cam. return NULL if not found.
    MultiThreadCapture* getCaptureForCam( CvCapture* cam );
    /// remove the MultiThreadCapture for the given cam.
    void removeCaptureForCam( CvCapture* cam );

private:

    typedef std::map<CvCapture* , MultiThreadCapture* > CaptureMap;
    CaptureMap capture_map;

    static MultiThreadCaptureManager* instance;
    FSemaphore lock;

};


class pyr_yape;
class keypoint;
class object_view;

class MultiThreadCapture : public FThread
{
public:
    /// caller retains ownership of the capture object
    MultiThreadCapture( CvCapture* capture );
    ~MultiThreadCapture();

    /// also process the image after capturing, into an image of the given
    /// width/height/bitdepth. if width is 0, don't process. retrieve with
    /// getLastProcessedFrame
    void setupResolution( int _width, int _height, int _nChannels, float desired_fps )
        { width = _width, height = _height; nChannels = _nChannels; desired_framerate = desired_fps; }

    /// Put a pointer to the last processed (grayscale) frame for the detect thread in
    /// grayFrame, and its last raw version in rawFrame; also put a
    /// timestamp for when the frame was grabbed into timestamp.
    ///
    /// If a new frame is not available, return false, unless
    /// block_until_available is true, in which case block until a new frame is
    /// available (or timeout and return false after 10s).
    ///
    /// You may pass NULL for grayFrame, rawFrame, or timestamp, and they
    /// will be ignored.
    ///
    /// The MultiThreadCapture instance keeps ownership of the returned
    /// pointers - they remain valid until the next call to getLastDetectFrame.
    /// Return true on success.
    bool getLastDetectFrame( IplImage** grayFrame, IplImage** rawFrame, FTime* timestamp_copy,
                               bool block_until_available = false );

    /// like getLastProcessedFrame but returns the last raw frame, rather than
    /// the last processed frame; maintains a separate available state for
    /// determining whether a frame is new or not.
    ///
    /// @see getLastDetectFrame
    bool getLastDrawFrame( IplImage** rawFrame, FTime* timestamp_copy,
                         bool block_until_available = false );

    /// Put a copy of the last frame captured into *last_frame_copy, and its timestamp
    /// into timestamp_copy (if non-NULL).
    ///
    /// If *last_frame_copy is NULL, a new IplImage will be constructed for you.
    /// If *last_frame_copy is not NULL, we check that the size, depth and chennels
    /// match. If they do, we copy frame data in and return true. If they don't, we
    /// return false.
    ///
    /// In any case, caller takes or retains ownership of *last_frame_copy.
    ///
    /// If a new frame is not available, return false, unless block_until_available
    /// is true, in which case block until a new frame is available (or timeout and
    /// return false after 10s).
    bool getCopyOfLastFrame( IplImage** last_frame_copy, FTime* timestamp_copy=NULL,
                            bool block_until_available = false );


    /// get the frame index for the requested ftime
    unsigned int getFrameIndexForTime( const FTime& timestamp );

    /// start capture
    void startCapture();

    /// stop capture
    void stopCapture();

    /// number of channels returned by the camera. if camera is yet to start, wait max 10s then timeout.
    int getNumChannelsRaw();

	/// enable low-fps low-cpu frame capture for idle times...
	void captureAtLowFps( bool do_low_fps ) { low_fps = do_low_fps; }

private:

    /// overridden from base
    void ThreadedFunction();

	/// process thread
	void startProcessThread();
	void stopProcessThread();
	/// for pthreads interface
	static void* processPthreadFunc( void* );
	/// the work actually happens here
	void processThreadedFunction();

    /// swap detect thread pointers
    void swapDetectPointers();
    void swapDrawPointers();

    CvCapture* capture;

	// for the capture threa
    FSemaphore capture_frame_lock;
    IplImage* capture_frame;

	// for the process thread
	bool process_thread_should_exit;
	pthread_t process_thread;
	FSemaphore process_thread_semaphore;

    // lock for the last frame
    FSemaphore last_frame_lock;
    // double buffered
    // How this works: when requested via getLastProcessedFrame, we return processed (& last_frame + timestamp)
    // and swap processed and processed_ret pointers (same for last_frame + timestamp).
    // This way, during capture the capture thread is always writing to processed, and the
    // external requester only has processed_ret.
    // We are assuming that the external requester operates single-threadedly.
    IplImage *last_frame, *last_frame_ret, *last_frame_draw, *last_frame_draw_ret, *last_frame_working;
    IplImage *processed, *processed_ret, *processed_working;
    IplImage *frame_processsize;
    FTime* timestamp, *timestamp_ret, *timestamp_draw, *timestamp_draw_ret, *timestamp_working;


    bool new_draw_frame_available, new_detect_frame_available;

    int width, height, nChannels, nChannelsRaw;
    float desired_framerate;
    FTime framerate_timer;

    typedef map<FTime, unsigned int> FTimeToFrameNumberIdx;
    FTimeToFrameNumberIdx ftime_framenum_idx;
    unsigned int frame_counter;

	bool low_fps;
};

