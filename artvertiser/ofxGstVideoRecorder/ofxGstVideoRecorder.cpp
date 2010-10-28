/*
 * ofxGstVideoRecorder.cpp
 *
 *  Created on: 14/01/2010
 *      Author: arturo
 */

#include "ofxGstVideoRecorder.h"
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>
//#include "Poco/Timestamp.h"

//Poco::Timestamp time_fps;
//Poco::Timestamp::TimeDiff one_frame_time;

string ofToString( int num ) { 
	char buf[256];
	sprintf( buf, "%i", num );
	return buf;
}

ofxGstVideoRecorder::ofxGstVideoRecorder() {
//	pixels_object=NULL;
	fps = 30;
	bIsTcpStream = false;
	bIsUdpStream = false;
}

void ofxGstVideoRecorder::shutdown()
{
    if(gstSrc) 
	    gst_app_src_end_of_stream (gstSrc);
}

ofxGstVideoRecorder::~ofxGstVideoRecorder() {
	shutdown();
}

void ofxGstVideoRecorder::udpStreamTo(string comma_separated_ips){
	sink  = " multiudpsink clients=" + comma_separated_ips;
	bIsUdpStream = true;
}

void ofxGstVideoRecorder::tcpStreamTo(string host, int port){
	sink = " tcpserversink host="+host+" port="+ofToString(port);
	bIsTcpStream = true;
}

void ofxGstVideoRecorder::setup(int width, int height, int bpp, string file, Codec codec, int fps){
	bLoaded      		= false;
	bPaused 			= true;
	speed 				= 1.0f;
	bHavePixelsChanged 	= false;
	bIsCamera = true;
	bIsCustomWithSink	= true;
	this->width = width;
	this->height = height;
	this->bpp = bpp;
	this->fps = fps;


	gstData.loop		= g_main_loop_new (NULL, FALSE);

	gst_debug_set_active (true);

	/*GstAppSrcCallbacks callbacks;
	callbacks.need_data = &gst_need_data;
	callbacks.enough_data = &gst_enough_data;
	callbacks.seek_data = &gst_seek_data;*/


	if(!bIsTcpStream && !bIsUdpStream && !bIsPipe)
		sink= "filesink name=video-sink sync=false location=" + file;

	string encoder;
	string muxer = "avimux ! ";
	string pay = "";
	string videorate = "videorate ! video/x-raw-rgb, framerate="+ofToString(fps)+"/1 ! ";

	switch(codec){
	case THEORA:
		encoder = "theoraenc quality=63 ! ";
		muxer = "oggmux !";
		pay = "rtptheorapay pt=96 !";
	break;
	case H264:
		encoder = "x264enc pass=4 !";
		pay = "rtph264pay pt=96 !";
	break;
	case MP4:
		encoder = "ffenc_mpeg4 ! ";
		pay = "rtpmp4vpay pt=96 !";
	break;
	case XVID:
		encoder = "xvidenc ! ";
	break;
	case JPEG:
		encoder = "jpegenc ! ";
		pay = "rtpjpegpay pt=96 !";
	break;
	case LOSLESS_JPEG:
		encoder = "ffenc_ljpeg ! ";
	break;
	case PDF:
		//videorate = "videorate ! video/x-raw-rgb, depth=24, bpp=32, endianness= 4321, red_mask= 65280, green_mask= 16711680, blue_mask= -16777216, framerate="+ofToString(fps)+"/1 !";
		encoder = "cairorender ! ";
		muxer = "";
	break;
	case PNG_SEQUENCE:
		encoder = "pngenc snapshot=false ! ";
		sink = "multifilesink name=video-sink location=" + file.substr(0,file.rfind('.')) + "%05d" + file.substr(file.rfind('.'));
		videorate = "";
		muxer = "";
	break;
	case QT_ANIM:
		encoder = "ffenc_qtrle ! ";
	break;
	case FLV:
		encoder = "ffenc_flv ! ";
		muxer = "flvmux ! ";
	break;
	case FLV_H264:
		encoder = "x264enc ! ";
		muxer = "flvmux ! ";
		pay = "rtph264pay pt=96 !";
	break;
	case YUV:
		encoder = "";
		muxer = "avimux ! ";
	break;
	case DIRAC:
		encoder = "schroenc ! ";
		//muxer = "ffmux_dirac ! ";
	break;
	}


	if(bIsUdpStream){
		muxer = "";
		if(pay!=""){
			pay = "queue ! " + pay;
		}else{
			printf( "this format doesnt support streaming, choose one of THEORA, H264, MP4, JPEG\n");
			return;
		}
	}else{
		pay   = "";
		if(bIsTcpStream){
			muxer = "mpegtsmux name=mux ! ";
		}
	}

	string input_mime;
	string other_format;
	if(bpp==24 || bpp==32){
		input_mime = "video/x-raw-rgb";
		other_format = ",endianness=4321,red_mask=255,green_mask=65280,blue_mask=16711680,framerate="+ofToString(fps)+"/1 ";
	}
	if(bpp==8){
		input_mime = "video/x-raw-gray";
		other_format = ",framerate="+ofToString(fps)+"/1";
	}

	if(src==""){
		src="appsrc  name=video_src is-live=true do-timestamp=true ! "
				+ input_mime
				+ ", width=" + ofToString(width)
				+ ", height=" + ofToString(height)
				+ ", depth=" + ofToString(bpp)
				+ ", bpp=" + ofToString(bpp)
				+ other_format;
	}


	string pipeline_string = src + " ! " +
									videorate +
									"queue ! ffmpegcolorspace ! " +
									 encoder + muxer +
									 pay + sink;

	printf( "gstreamer pipeline: %s\n", pipeline_string.c_str());

	GError * error = NULL;
	gstPipeline = gst_parse_launch (pipeline_string.c_str(), &error);
	if ( !gstPipeline || error )
		printf("parse launch error: %s\n", error->message );

	gstSink = gst_bin_get_by_name(GST_BIN(gstPipeline),"video-sink");
	gstSrc = (GstAppSrc*)gst_bin_get_by_name(GST_BIN(gstPipeline),"video_src");
	if(gstSrc){
		gst_app_src_set_stream_type (gstSrc,GST_APP_STREAM_TYPE_STREAM);
		g_object_set (G_OBJECT(gstSrc), "format", GST_FORMAT_TIME, NULL);
		//gst_app_src_set_max_bytes(gstSrc, 30 * width*height*bpp/8);
		//g_object_set (G_OBJECT(gstSrc), "block", TRUE, NULL);
		//gst_app_src_set_size (gstSrc,10000*width*height*bpp/8);
		//gst_app_src_set_callbacks (gstSrc, &callbacks, &data, &gst_destroy_notify);
	}
	else
	{
		printf("got no gstSrc\n");
	}

	pixels = new unsigned char[width*height*bpp/8];
	data.pixels=pixels;
	data.source=gstSrc;
	data.width = width;
	data.height = height;
	data.bpp = bpp;
	data.new_frame = true;
	data.fps = fps;

	//one_frame_time = 1000000/fps;

	/*GstElement *audioSink = gst_element_factory_make("gconfaudiosink", NULL);
	g_object_set (G_OBJECT(gstPipeline),"audio-sink",audioSink,(void*)NULL);*/

	startPipeline();
	play();

}

/*
void ofxGstVideoRecorder::setup(ofBaseImage & pixels,int bpp, string file, Codec codec,int fps){
	pixels_object = &pixels;
	ofAddListener(ofEvents.update,this,&ofxGstVideoRecorder::update);
	setup(pixels.getWidth(),pixels.getHeight(),bpp,file,codec,fps);
}

void ofxGstVideoRecorder::setup(ofBaseVideo & video,int bpp, string file, Codec codec,int fps){
	video_object = &video;
	ofAddListener(ofEvents.update,this,&ofxGstVideoRecorder::update);
	setup(video.getWidth(),video.getHeight(),bpp,file,codec,fps);
}

*/
void ofxGstVideoRecorder::setupRecordWindow(int x, int y, int width, int height,int bpp, string file, Codec codec, int fps){
	int type=0;
/*	if(bpp==8)
		type=OF_IMAGE_GRAYSCALE;
	else if(bpp==24)
		type=OF_IMAGE_COLOR;
	else if(bpp==32)
		type=OF_IMAGE_COLOR_ALPHA;
	window_img.allocate(width,height,type);
*/
	win_x = x;
	win_y = y;
	bRecordWindow = true;
//	ofAddListener(ofEvents.update,this,&ofxGstVideoRecorder::update);
	setup(width,height,bpp,file,codec,fps);
}


void ofxGstVideoRecorder::setupRecordScreen(int x, int y, int width, int height,int bpp, string file, Codec codec, int fps){
	src = "ximagesrc show-pointer=true startx="+ofToString(x)+" starty="+ofToString(y)+" endx="+ofToString(x+width)+" endy="+ofToString(y+width)+ " use-damage=false ! video/x-raw-rgb, framerate=30/1";
	setup(width,height,bpp,file,codec,fps);
}

void ofxGstVideoRecorder::newFrame(unsigned char * pixels){
	GstBuffer * buffer;
	buffer = gst_app_buffer_new (pixels, width*height*bpp/8, NULL, pixels);

	GstFlowReturn flow_return = gst_app_src_push_buffer(gstSrc, buffer);
	if (flow_return != GST_FLOW_OK) {
		printf("error pushing buffer: flow_return was %i\n", flow_return );
	}
	ofGstUtils::update();
}
/*
void ofxGstVideoRecorder::update(ofEventArgs & voidArgs){
	if(video_object){
		if(video_object->isFrameNew()) newFrame(video_object->getPixels());
	}

	if(time_fps.elapsed()>=one_frame_time) {
		time_fps.update();

		if(pixels_object){
			newFrame(pixels_object->getPixels());
		}else if(bRecordWindow){
			window_img.grabScreen(win_x,win_y,width,height);
			newFrame(window_img.getPixels());
		}
	}
}
*/
