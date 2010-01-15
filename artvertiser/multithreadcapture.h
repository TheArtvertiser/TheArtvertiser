
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

class MultiThreadCapture : public FThread
{
public:
    /// caller retains ownership of the capture object
    MultiThreadCapture( CvCapture* capture );
    ~MultiThreadCapture();

    /// also process the image after capturing, into an image of the given
    /// width/height/bitdepth. if width is 0, don't process. retrieve with
    /// getLastProcessedFrame
    void setupResolution( int _width, int _height, int _nChannels )
        { width = _width, height = _height; nChannels = _nChannels; }

    /// Put a pointer to the last processed frame in processedFrame, and its
    /// last raw version in rawFrame; also put a timestamp for when the
    /// frame was grabbed into timestamp. The MultiThreadCapture instance
    /// keeps ownership of the returned pointers - they remain valid until the
    /// next call to getLastProcessedFrame. Return true on success.
    /// If a new frame is not available, return false, unless
    /// block_until_available is true, in which case block until a new frame is
    /// available (or timeout and return false after 10s).
    ///
    /// You may pass NULL for processedFrame, rawFrame, or timestamp, and they
    /// will be ignored.
    bool getLastProcessedFrame( IplImage** processedFrame, IplImage** rawFrame, FTime** timestamp, bool block_until_available = false );

    /// Put a copy of the last frame returned by cvQueryFrame into *last_frame_copy,
    /// and its timestamp into timestamp_copy (if non-NULL).
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
    bool getCopyOfLastFrame( IplImage** last_frame_copy, FTime* timestamp_copy=NULL, bool block_until_available = false );

    /// start capture
    void startCapture();

    /// stop capture
    void stopCapture();

private:

    /// overridden from base
    void ThreadedFunction();

    /// swap image pointers
    void swapImagePointers();

    CvCapture* capture;

    // lock for the last frame
    FSemaphore last_frame_lock;
    IplImage *last_frame, *last_frame_ret;
    IplImage *processed, *processed_ret;
    IplImage *frame_processsize;
    FTime* timestamp, *timestamp_ret;

    bool new_raw_frame_available, new_processed_frame_available;
    bool should_stop_capture;

    int width, height, nChannels;

};


