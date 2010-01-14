
#pragma once

#include <pthread.h>
#include <stdio.h>
#include "FProfiler/FThread.h"
#include "FProfiler/FSemaphore.h"
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

    /// put a pointer to the last processed frame in processedFrame,
    /// and its last raw version in rawFrame. the MultiThreadCapture instance
    /// keeps ownership of the returned pointers - they are valid until the
    /// next call to getLastProcessedFrame. You may pass NULL for either
    /// argument.
    /// if a new frame is not available, put NULL into *processedFrame and
    /// *rawFrame, unless block_until_available is true, in which case block
    /// until a new frame is available.
    void getLastProcessedFrame( IplImage** processedFrame, IplImage** rawFrame, bool block_until_available = false );

    /// return a COPY of the last frame returned by cvQueryFrame.
    /// caller must take ownership of the copy.
    /// returns NULL if no frame available.
    IplImage* getCopyOfLastFrame();


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

    bool new_frame_available;
    bool should_stop_capture;

    int width, height, nChannels;

};


