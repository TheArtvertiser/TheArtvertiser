/*
 Copyright 2005, 2006 Computer Vision Lab,
 Ecole Polytechnique Federale de Lausanne (EPFL), Switzerland.
 Modified by Damian Stewart <damian@frey.co.nz> 2009-2010;
 modifications Copyright 2009, 2010 Damian Stewart <damian@frey.co.nz>.

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

#include <iostream>
using namespace std;

#include <starter.h>
#include "object_view.h"
#include "../../artvertiser/FProfiler/FSemaphore.h"
#include "../../artvertiser/FProfiler/FProfiler.h"

// Constructor for training stage:
object_view::object_view(PyrImage * _image) :
                image(cvCreateImage(cvGetSize(_image->images[0]), IPL_DEPTH_8U, 1), _image->nbLev),
                gradX(cvCreateImage(cvGetSize(_image->images[0]), IPL_DEPTH_16S, 1), _image->nbLev),
                gradY(cvCreateImage(cvGetSize(_image->images[0]), IPL_DEPTH_16S, 1), _image->nbLev)
{
}

// Constructor for recognition stage (alloc memory once):
object_view::object_view(int width, int height, int nbLev) :
                image(cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1), nbLev),
                gradX(cvCreateImage(cvSize(width, height), IPL_DEPTH_16S, 1), nbLev),
                gradY(cvCreateImage(cvSize(width, height), IPL_DEPTH_16S, 1), nbLev)
{
}

void object_view::build_from_image_0(int kernelSize)
{
  if (kernelSize == 0)
    image.build();
  else
  {
    image.smoothLevel0(kernelSize);
    image.build();
  }
  comp_gradient();
}

void object_view::build(IplImage *im, int kernelSize)
{
  if (kernelSize == 0)
    cvCopy(im, image[0]);
  else if (kernelSize < 0)
    cvSmooth(im, image[0], CV_GAUSSIAN, 3, 3);
  else
    cvSmooth(im, image[0], CV_GAUSSIAN, kernelSize, kernelSize);
  image.build();
  comp_gradient();
}


#include <pthread.h>

class object_view::CompGradientThreadData
{
public:
    CompGradientThreadData() : semaphore(0), should_stop(false) {};
    ~CompGradientThreadData() { should_stop = true; semaphore.Signal(); void* ret; pthread_join( thread, &ret ); }

    pthread_t thread;
    FBarrier* shared_barrier;
    FSemaphore semaphore;
    bool should_stop;

    PyrImage* image;
    PyrImage* gradX;
    PyrImage* gradY;
    bool doX;

};




void* object_view::comp_gradient_thread_func( void* _data )
{
    CompGradientThreadData* data = (CompGradientThreadData*)_data;

    while ( true )
    {
        data->semaphore.Wait();
        if ( data->should_stop )
            break;

        for ( int l=0; l<data->image->nbLev; ++l )
        {
            if ( data->doX )
                cvSobel( data->image->operator[](l), data->gradX->operator[](l), 1, 0, 1 );
            else
                cvSobel( data->image->operator[](l), data->gradY->operator[](l), 0, 1, 1 );
        }

        data->shared_barrier->Wait();
    }

    pthread_exit(0);

    return NULL;

}


void object_view::comp_gradient_mt()
{
    PROFILE_THIS_FUNCTION();
    int num_threads = 2;
    if ( comp_gradient_thread_data.size() != num_threads )
    {
        assert( comp_gradient_thread_data.size()==0 );

        // construct shared barrier
        shared_barrier = new FBarrier( num_threads+1 );

        // construct threads
        printf("creating %i threads for object_view::comp_gradient\n", num_threads );
        pthread_attr_t thread_attr;
        pthread_attr_init(&thread_attr);
        pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
        for ( int i=0; i<num_threads; i++ )
        {
            CompGradientThreadData* thread_data = new CompGradientThreadData();
            thread_data->shared_barrier = shared_barrier;
            thread_data->image = &image;
            thread_data->gradX = &gradX;
            thread_data->gradY = &gradY;
            thread_data->doX = (i%2==0);
            // store
            comp_gradient_thread_data.push_back( thread_data );
            // launch thread
            pthread_create( &thread_data->thread, &thread_attr, &comp_gradient_thread_func, (void*)thread_data );
        }
    }


    // go
    for ( int i=0; i<comp_gradient_thread_data.size(); i++ )
    {
        CompGradientThreadData* thread_data = comp_gradient_thread_data[i];
        assert( thread_data->image == &image );
        assert( thread_data->gradX == &gradX );
        assert( thread_data->gradY == &gradY );
        thread_data->semaphore.Signal();
    }

    // wait for all to complete
    shared_barrier->Wait();

}

void object_view::comp_gradient()
{
    PROFILE_THIS_FUNCTION();
  for (int l = 0; l < image.nbLev; ++l) {

    cvSobel(image[l], gradX[l], 1, 0, 1);
    cvSobel(image[l], gradY[l], 0, 1, 1);
  }
}

object_view::~object_view()
{
    if ( comp_gradient_thread_data.size() > 0 )
    {
        for ( int i=0; i<comp_gradient_thread_data.size(); i++ )
            delete comp_gradient_thread_data[i];
        comp_gradient_thread_data.clear();

        delete shared_barrier;
    }
}

