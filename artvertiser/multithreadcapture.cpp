
#include "multithreadcapture.h"
#include <unistd.h>

#include "FProfiler/FProfiler.h"

MultiThreadCaptureManager* MultiThreadCaptureManager::instance = NULL;

MultiThreadCapture* MultiThreadCaptureManager::getCaptureForCam( CvCapture* cam )
{
    MultiThreadCapture* result = 0;
    lock.Wait();
    if ( capture_map.find( cam ) != capture_map.end() )
        result = capture_map[cam];
    lock.Signal();
    return result;
}

void MultiThreadCaptureManager::addCaptureForCam(CvCapture* cam, MultiThreadCapture* cap )
{
    lock.Wait();
    capture_map[cam] = cap;
    lock.Signal();
}

void MultiThreadCaptureManager::removeCaptureForCam( CvCapture* cam )
{
    lock.Wait();
    capture_map.erase( cam );
    lock.Signal();
}

MultiThreadCapture::MultiThreadCapture( CvCapture* _capture )
: capture (_capture), FThread(), last_frame(NULL), should_stop_capture(false), width(0), height(0), nChannels(0),
frame_processsize( 0 ), processed( 0 ), last_frame_ret(0), processed_ret(0), new_frame_available(false)
{
    MultiThreadCaptureManager* manager = MultiThreadCaptureManager::getInstance();
    assert( manager->getCaptureForCam( capture ) == NULL );
    manager->addCaptureForCam( capture, this );

}

MultiThreadCapture::~MultiThreadCapture()
{
    printf("in ~MultiThreadCapture\n");
    stopCapture();

    /*if ( last_frame )
        cvRelease( &last_frame );*/
    last_frame = NULL;

    MultiThreadCaptureManager* manager = MultiThreadCaptureManager::getInstance();
    assert( manager->getCaptureForCam( capture ) != NULL );
    manager->removeCaptureForCam( capture );
}


void MultiThreadCapture::startCapture()
{
    if ( thread_running )
    {
        printf("MultiThreadCapture(%x)::StartCapture(): capture already running\n", this );
        return;
    }
    new_frame_available=false;

    FThread::StartThread();
}

void MultiThreadCapture::stopCapture()
{
    if ( !thread_running )
        return;

    should_stop_capture = true;
    // sleep until thread has stopped
    int timeout = 1000;
    while ( thread_running && timeout > 0 )
    {
        usleep( 10000 );
        timeout -= 10;
    }
    if ( timeout <= 0 )
    {
        printf("MultiThreadCapture %x:StopCapture(): warning, thread stop timed out\n", this );
    }
    should_stop_capture = false;

}

void MultiThreadCapture::ThreadedFunction()
{
    while ( !should_stop_capture )
    {
        // try to get the frame
        if ( cvGrabFrame( capture ) )
        {
            IplImage* f = cvRetrieveFrame( capture );
            if ( f )
            {
                // got! now process
                last_frame_lock.Wait();

                if ( last_frame==0 || last_frame->width != f->width || last_frame->height != f->height || last_frame->depth != f->depth )
                {
                    if ( last_frame )
                        cvReleaseImage( &last_frame );
                    last_frame = cvCreateImage( cvGetSize( f ) , IPL_DEPTH_8U, f->nChannels );
                }
                // store locally
                cvCopy( f, last_frame );

                // process
                if ( width != 0 )
                {
                    PROFILE_SECTION_PUSH("mtc frame processing");
                    CvSize process_size = cvSize( width, height );
                    CvSize frame_size = cvGetSize( last_frame );

                    // ok
                    // if framesize == processsize and nChannels correct, then processed=frame
                    // if framesize != processsize and nChannels correct, resize frame->processed

                    // if framesize == processsize and nChannels wrong,         framedetect=frame then cvtcolor framedetect->processed
                    // if framesize != processsize and nChannels wrong, resize frame->framedetect then cvtcolor framedetect->processed

                    // already processed?
                    bool frame_process_same_size = (frame_size.width == process_size.width && frame_size.height == process_size.height);
                    if ( last_frame->nChannels == nChannels )
                    {
                        if ( frame_process_same_size )
                            processed = last_frame;
                        else
                        {
                            PROFILE_SECTION_PUSH( "resize" );
                            if ( !processed )
                                processed = cvCreateImage( cvSize( process_size.width, process_size.height ), IPL_DEPTH_8U, nChannels );
                            cvResize( last_frame, processed );
                            PROFILE_SECTION_POP();
                        }
                    }
                    else
                    {
                        PROFILE_SECTION_PUSH( "resize" );
                        if ( frame_process_same_size )
                            frame_processsize = last_frame;
                        else
                        {
                            if ( !frame_processsize || frame_processsize->width != process_size.width || frame_processsize->height != process_size.height )
                            {
                                if ( frame_processsize )
                                    cvReleaseImage( &frame_processsize );
                                frame_processsize = cvCreateImage( process_size, IPL_DEPTH_8U, f->nChannels );
                            }
                            cvResize( last_frame, frame_processsize );
                        }
                        PROFILE_SECTION_POP();

                        PROFILE_SECTION_PUSH( "convert colors" );
                        if ( !processed )
                            processed = cvCreateImage( cvSize( process_size.width, process_size.height ), IPL_DEPTH_8U, nChannels );
                        int convert = (nChannels == 1?CV_RGBA2GRAY:CV_GRAY2RGBA);
                        cvCvtColor( frame_processsize, processed, convert );
                        PROFILE_SECTION_POP();
                    }
                    PROFILE_SECTION_POP();
                }

                // we have a new frame
                new_frame_available = true;

                last_frame_lock.Signal();

            }
        }
        else
        {
            printf("cvGrabFrame failed: trying to rewind\n");
            // try rewinding
            cvSetCaptureProperty( capture, CV_CAP_PROP_POS_FRAMES, 0 );
        }

        // wait a bit
        usleep( 10000 );
    }
}

IplImage* MultiThreadCapture::getCopyOfLastFrame()
{
    IplImage* ret;
    // lock
    last_frame_lock.Wait();
    // have a last frame?
    if ( last_frame == NULL )
        ret = NULL;
    else
        ret = cvCloneImage( last_frame );
    last_frame_lock.Signal();
    return ret;
}

void MultiThreadCapture::getLastProcessedFrame( IplImage** processedFrame, IplImage** rawFrame, bool block_until_available )
{
    // handle frame availability
    if ( !new_frame_available )
    {
        if ( block_until_available )
        {
            int timeout_us = 500*1000; // 1 second
            while ( !new_frame_available && timeout_us > 0 )
            {
                usleep( 100 );
                timeout_us -= 100;
            }
            if ( !new_frame_available )
            {
                printf("capture timed out\n");
                *processedFrame = NULL;
                *rawFrame = NULL;
                return;
            }
        }
        else
        {
            *processedFrame = NULL;
            *rawFrame = NULL;
            return ;
        }
    }

    last_frame_lock.Wait();
    // get image pointers
    if ( processedFrame != NULL )
        *processedFrame = processed;
    if ( rawFrame != NULL )
        *rawFrame = last_frame;
    // swap pointers to keep processed images valid until the next call
    swapImagePointers();
    new_frame_available = false;
    last_frame_lock.Signal();
}

void MultiThreadCapture::swapImagePointers()
{
    IplImage* temp = last_frame;
    last_frame = last_frame_ret;
    last_frame_ret = temp;

    temp = processed;
    processed = processed_ret;
    processed_ret = processed;

}
