
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
frame_processsize( 0 ), processed( 0 ), last_frame_ret(0), processed_ret(0), new_raw_frame_available(false), new_processed_frame_available(false), timestamp(0), timestamp_ret(0)
{
    MultiThreadCaptureManager* manager = MultiThreadCaptureManager::getInstance();
    assert( manager->getCaptureForCam( capture ) == NULL );
    manager->addCaptureForCam( capture, this );

}

MultiThreadCapture::~MultiThreadCapture()
{
    printf("in ~MultiThreadCapture\n");
    stopCapture();

    // release working space for frame capture
    if ( frame_processsize && frame_processsize != last_frame && frame_processsize != last_frame_ret
        && frame_processsize != processed && frame_processsize != processed_ret )
    {
        cvReleaseImage( &frame_processsize );
        frame_processsize = NULL;
    }
    // release frame
    if ( last_frame )
    {
        cvReleaseImage( &last_frame );
        cvReleaseImage( &processed );
        delete timestamp;
        last_frame = NULL;
    }
    // release swapped frame
    if ( last_frame_ret )
    {
        cvReleaseImage( &last_frame_ret );
        cvReleaseImage( &processed_ret );
        delete timestamp_ret;
        last_frame_ret = NULL;
    }

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
    new_raw_frame_available=false;
    new_processed_frame_available=false;

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

                // store timestamp
                if ( timestamp == 0 )
                {
                    timestamp = new FTime();
                }
                timestamp->SetNow();

                // store locally
                if ( last_frame==0 || last_frame->width != f->width || last_frame->height != f->height || last_frame->depth != f->depth )
                {
                    if ( last_frame )
                        cvReleaseImage( &last_frame );
                    last_frame = cvCreateImage( cvGetSize( f ) , IPL_DEPTH_8U, f->nChannels );
                }
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
                        /*PROFILE_SECTION_PUSH( "equalize hist" );
                        cvEqualizeHist( processed, processed );
                        PROFILE_SECTION_POP();*/
                    }
                    PROFILE_SECTION_POP();
                }

                // we have a new frame
                new_raw_frame_available = true;
                new_processed_frame_available = true;

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


bool MultiThreadCapture::getCopyOfLastFrame( IplImage** last_frame_copy, FTime* timestamp_copy, bool block_until_available )
{
    // block until new frame is available
    if ( !new_raw_frame_available )
    {
        if ( !block_until_available )
            return false;
        else
        {
            //printf("blocking until new frame available\n");
            int timeout_us = 10000*1000; // 10000ms = 10s
            while ( !new_raw_frame_available && timeout_us > 0 )
            {
                usleep( 100 );
                timeout_us -= 100;
            }
            if ( !new_raw_frame_available )
            {
                printf("capture timed out\n");
                return false;
            }
        }
    }
    // lock
    last_frame_lock.Wait();
    bool ret = true;
    // have a last frame?
    if ( last_frame == NULL )
    {
        *last_frame_copy = NULL;
        ret = false;
    }
    else
    {
        // copy the frame into target

        // check dimensions
        if ( *last_frame_copy )
        {
            if (
                (*last_frame_copy)->width != last_frame->width ||
                (*last_frame_copy)->height != last_frame->height ||
                (*last_frame_copy)->nChannels != last_frame->nChannels ||
                (*last_frame_copy)->depth != last_frame->depth )
            {
                fprintf(stderr, "input size doesn't match: last_frame is %ix%i %i %i, but we were given %ix%i %i %i",
                        last_frame->width, last_frame->height, last_frame->depth, last_frame->nChannels,
                        (*last_frame_copy)->width, (*last_frame_copy)->height, (*last_frame_copy)->depth, (*last_frame_copy)->nChannels );
                return false;
            }
        }
        // construct, if NULL has been passed in
        else /*( *last_frame_copy == NULL )*/
        {
            *last_frame_copy = cvCreateImage( cvGetSize( last_frame ), last_frame->depth, last_frame->nChannels );
        }
        cvCopy( last_frame, *last_frame_copy );

        if ( timestamp_copy )
            // invoke assignment operator=
            *timestamp_copy = *timestamp;

        // raw frame no longer available
        new_raw_frame_available = false;
    }
    last_frame_lock.Signal();
    return ret;
}

bool MultiThreadCapture::getLastProcessedFrame( IplImage** processedFrame, IplImage** rawFrame, FTime** timeStamp, bool block_until_available )
{
    // handle frame availability
    if ( !new_processed_frame_available )
    {
        if ( block_until_available )
        {
            int timeout_us = 10000*1000; // 10000ms = 10s
            while ( !new_processed_frame_available && timeout_us > 0 )
            {
                usleep( 100 );
                timeout_us -= 100;
            }
            if ( !new_processed_frame_available )
            {
                printf("capture timed out\n");
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    if ( last_frame )
    {
        last_frame_lock.Wait();
        // get image pointers
        if ( processedFrame )
            *processedFrame = processed;
        if ( rawFrame )
            *rawFrame = last_frame;
        if ( timeStamp )
            *timeStamp = timestamp;
        // swap pointers to keep processed images valid until the next call
        swapImagePointers();
        new_processed_frame_available = false;
        new_raw_frame_available = false;
        last_frame_lock.Signal();
        return true;
    }
    else
        return false;
}

void MultiThreadCapture::swapImagePointers()
{
    IplImage* temp = last_frame;
    last_frame = last_frame_ret;
    last_frame_ret = temp;

    temp = processed;
    processed = processed_ret;
    processed_ret = processed;

    FTime* temp_t = timestamp;
    timestamp = timestamp_ret;
    timestamp_ret = temp_t;

}
