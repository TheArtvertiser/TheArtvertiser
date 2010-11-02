/*
 *  avImage.cpp
 *  artvertiser
 *
 *  Created by damian on 05/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "avImage.h"

#include <map>

#include "ofxOpenCv.h"

#define VIDEO_SWAP_RED_BLUE

typedef map< ofBaseVideo*, IplImage* > WorkingFrames;

class WorkingFramesDatabase
{
public:

	IplImage*& operator[] ( ofBaseVideo* source )
	{
		return working[ source ];
	}
	WorkingFrames::iterator find( ofBaseVideo* source )
	{
		return working.find( source );
	}
	WorkingFrames::iterator end()
	{
		return working.end();
	}

	void erase( ofBaseVideo* source )
	{
		working.erase( source );
	}

	WorkingFrames working;

};

WorkingFramesDatabase working_frames;

IplImage* avGetFrame( ofBaseVideo* video_source )
{
	// get a frame from the video source

	bool allocate = false;
	IplImage* working_frame = NULL;

	// do we have one?
	if ( working_frames.find( video_source )== working_frames.end() )
	{
		allocate = true;
	}
	else
	{
		// is the one we have the right size?
		working_frame = working_frames[video_source];
		CvSize size = cvGetSize( working_frame );
		if ( size.width != video_source->getWidth() ||
			size.height != video_source->getHeight() )
		{
			working_frames.erase( video_source );
			cvReleaseImage( &working_frame );
			allocate = true;
		}
	}

	// allocate
	if ( allocate )
	{
		// 24 bit RGB
		working_frame = cvCreateImage( cvSize( video_source->getWidth(), video_source->getHeight() ),
									  IPL_DEPTH_8U, 3 );
		//strcpy( working_frame->channelSeq, "BGR" );
		//strcpy( working_frame->channelSeq, "RGB" );
		working_frames[video_source] = working_frame;
	}

	// now copy
	unsigned char* pixels = video_source->getPixels();
	int w = video_source->getWidth();
	int h = video_source->getHeight();
	if( working_frame->width == working_frame->widthStep ){
		memcpy( working_frame->imageData,  pixels, w*h*3);
	}else{
		for( int i=0; i < h; i++ ) {
			char* target_line_start = working_frame->imageData + (i*working_frame->widthStep);
			memcpy( target_line_start,
				   pixels + (i*w*3),
				   w*3 );
		}
	}

#ifdef VIDEO_SWAP_RED_BLUE
	for ( int i=0; i<h ;i++ )
	{
		unsigned char* base = (unsigned char*)working_frame->imageData + (i*working_frame->widthStep);
		for ( int j=0; j<w; j++ )
		{
			unsigned char* data = base + j*3;
			// swap red/blue
			data[0] ^= data[2];
			data[2] ^= data[0];
			data[0] ^= data[2];

			/*
			unsigned char tmp = data[0];
			data[0] = data[2];
			data[2] = tmp;*/
		}
	}
#endif
	return working_frame;
}



void toOfImage( IplImage* source, ofImage& dest )
{
	IplImage* f = source;
#ifdef VIDEO_SWAP_RED_BLUE
	bool order_is_rgb = false;
#else
	bool order_is_rgb = true;
#endif

	int type = OF_IMAGE_COLOR;
	if ( f->nChannels == 1 )
		type = OF_IMAGE_GRAYSCALE;
	else if ( f->nChannels == 4 )
		type = OF_IMAGE_COLOR_ALPHA;
	if ( f->imageData != NULL )
		dest.setFromPixels( (unsigned char*)f->imageData, f->width, f->height, type, order_is_rgb );
}


int avSaveImage( const char* path, CvArr* image )
{
	static ofImage working;
	working.setUseTexture( false );
	printf("saving image %ix%i to %s\n", ((IplImage*)image)->width, ((IplImage*)image)->height, path );
	toOfImage( (IplImage*)image, working );
	working.saveImage( path );
	return true;
}


IplImage* avLoadImage( const char* path, int is_color )
{
	FILE* check_exists = fopen( path, "r" );
	if ( check_exists == NULL )
	{
		printf("avLoadImage: couldn't open file '%s' (does it exist?)\n", path );
		return NULL;
	}
	fclose( check_exists );

	static ofImage working;
	working.setUseTexture( false );
	working.loadImage( path );
	if ( working.width == 0 )
	{
		printf("avLoadImage couldn't load '%s'\n", path );
		return NULL;
	}
	else
		printf("avLoadImage loaded %ix%i %s %s\n", working.width, working.height, working.type==OF_IMAGE_GRAYSCALE?"OF_IMAGE_GRAYSCALE":
			   (working.type==OF_IMAGE_COLOR?"OF_IMAGE_COLOR":"OF_IMAGE_COLOR_ALPHA"), path );

	int n_channels;
	// take care of 'automatic'
	if ( is_color == -1 )
	{
		if ( working.type==OF_IMAGE_GRAYSCALE )
			is_color = 0;
		else
			is_color = 1;
	}
	if ( is_color == 1 )
		n_channels = 3;
	else if ( is_color == 0 )
		n_channels = 1;
	else
		n_channels = ( working.type == OF_IMAGE_GRAYSCALE ? 1:3 );

	IplImage* output = cvCreateImage( cvSize( working.width, working.height ), IPL_DEPTH_8U, n_channels );

	// copy from working to output
	unsigned char* pixels = working.getPixels();
	int w = working.getWidth();
	int h = working.getHeight();

	if ( is_color == 1 && working.type != OF_IMAGE_COLOR )
	{
		// convert to color
		if ( working.type == OF_IMAGE_COLOR_ALPHA )
		{
			// throw away alpha channel
			for ( int i=0; i<h ;i++ )
			{
				unsigned char* source_base = pixels + (i*4*w);
				unsigned char* target_base = (unsigned char*)output->imageData + i*output->widthStep;
				for ( int j=0; j<w; j++ )
				{
					unsigned char* source_data = source_base + j*4;
					unsigned char* target_data = target_base + j*3;
					memcpy( target_data, source_data, 3 );
				}
			}
		}
		else
		{
			// convert from grayscale
			for ( int i=0; i<h ;i++ )
			{
				unsigned char* source_base = pixels + (i*w);
				unsigned char* target_base = (unsigned char*)output->imageData + i*output->widthStep;
				for ( int j=0; j<w; j++ )
				{
					unsigned char* source_data = source_base + j;
					unsigned char* target_data = target_base + j*3;
					target_data[0] = source_data[0];
					target_data[1] = source_data[0];
					target_data[2] = source_data[0];
				}
			}
		}
	}
	else if ( is_color == 0 && working.type != OF_IMAGE_GRAYSCALE )
	{
		// convert to gray
		int bpp = 3;
		if ( working.type == OF_IMAGE_COLOR_ALPHA )
			// alpha is discarded
			bpp = 4;

		for ( int i=0; i<h ;i++ )
		{
			unsigned char* source_base = pixels + i*bpp*w;
			unsigned char* target_base = (unsigned char*)output->imageData + i*output->widthStep;
			for ( int j=0; j<w; j++ )
			{
				unsigned char* source_data = source_base + j*bpp;
				unsigned char* target_data = target_base + j;
				target_data[0] = (unsigned char)min(255.0f,
													0.2126f*source_data[0] +
													0.7152f*source_data[1] +
													0.0722f*source_data[2]);
			}
		}
	}
	else // pass color through
	{
		int bypp = n_channels;
		// straight copy
		if( output->width == output->widthStep ){
			memcpy( output->imageData,  pixels, w*h*bypp);
		}else{
			for( int i=0; i < h; i++ ) {
				char* target_line_start = output->imageData + (i*output->widthStep);
				memcpy( target_line_start,
					   pixels + (i*w*bypp),
					   w*bypp );
			}
		}
	}

	return output;
}


string fromOfDataPath( string path )
{
	string baseDataFolder = ofToDataPath(".");
	baseDataFolder = baseDataFolder.substr(0,baseDataFolder.size()-1);
	size_t pos = path.find( baseDataFolder );
	if ( pos == string::npos || baseDataFolder.size() >= path.size() )
		return path;
	else
		return path.substr( pos+baseDataFolder.size() );
}


string toAbsolutePath( string path )
{
	if ( path.size()==0 || path[0] == '/' )
		return path;

	// prepend CWD to passed-in path
	char cwd[2048];
	getcwd( cwd, 2048 );
	return string(cwd)+"/"+path;
}

string fromAbsolutePath( string path )
{
	if ( path.size()==0 )
		return path;

	// search for CWD in passed-in path
	char cwd[2048];
	getcwd( cwd, 2048 );
	size_t pos = path.find( cwd );
	if ( pos == string::npos || strlen(cwd) >= path.size() )
		return path;
	else
		return path.substr( pos+strlen(cwd)+1 );

}


string fromOfDataOrAbsolutePath( string path )
{
	string model_path = fromOfDataPath( path );
	// only assume data/ relative if fromOfDataPath returns something different
	if ( model_path != path )
		model_path = string("data/")+model_path;
	else
		model_path = fromAbsolutePath( path );

	return model_path;
}