/*
 *  Artvert.cpp
 *  artvertiser
 *
 *  Created by damian on 19/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "Artvert.h"

vector<ofImage*> Artvert::fallback_artvert_images;


Artvert::Artvert()
{ 
	model_file="<uninitialised>"; 
	artvert_image_file="<uninitialised>"; 
	type = AV_NONE;
	artist = "unknown artist";
	advert_name = "unknown advert";
	title = "untitled artvert";
	avi_capture = NULL;
	active = false;
	
	// do we need to initialise?
	if ( fallback_artvert_images.size()==0 )
	{
		printf("loading fallback artvert images from data/...\n");
		// load fallback images
		// default, should always be there
		ofImage* image = new ofImage();
		bool loaded = image->loadImage("fallback_artvert_image.png");
		assert( loaded && "couldn't load data/fallback_artvert_image.png" );
		fallback_artvert_images.push_back( image );
		// optional, fallback_artvert_image_[0..9].png
		for ( int i=0; i<10; i++ )
		{
			char buf[256];
			sprintf(buf, "fallback_artvert_image_%i.png", i );
			image = new ofImage();
			loaded = image->loadImage( buf );
			if ( loaded )
				fallback_artvert_images.push_back( image );
		}
		
	}
	
	which_fallback_image = -1;
}


Artvert::~Artvert()
{
	shutdown();
}

void Artvert::shutdown()
{	
	deactivate();
	
	if ( avi_capture )
		delete avi_capture;
	avi_capture = NULL;
}


void Artvert::activate()
{
	printf("artvert '%s': activate() called\n", getDescription().c_str() );
	if ( active )
		return;
	
	if ( type == AV_MOVIE )
	{
		if ( avi_capture == NULL )
		{
			avi_capture = new ofVideoPlayer();
			if ( avi_capture->loadMovie( artvert_movie_file.c_str() ) )
			{
				avi_capture->setLoopState( OF_LOOP_NORMAL );
			}
			else
				ofLog( OF_LOG_ERROR, "failed to load movie '%s'", artvert_movie_file.c_str() );
		}	
		
		if ( avi_capture->width != 0 )
		{
			avi_capture->play();
			avi_capture->setVolume(0);
			texture.allocate( avi_capture->width, avi_capture->height, GL_RGB );
		}

	}
	else if ( type == AV_IMAGE )
	{
		if ( !texture.bAllocated() )
		{
			printf("loading artvert image '%s'\n", artvert_image_file.c_str() );
			ofImage artvert_image;
			artvert_image.setUseTexture(false);
			if ( artvert_image.loadImage( artvert_image_file.c_str() ) )
			{	
				GLuint gl_type;
				if ( artvert_image.type == OF_IMAGE_COLOR )
					gl_type = GL_RGB;
				else if ( artvert_image.type == OF_IMAGE_COLOR_ALPHA )
					gl_type = GL_RGBA;
				else
					gl_type = GL_LUMINANCE;
				texture.allocate( artvert_image.getWidth(), artvert_image.getHeight(), gl_type );
				texture.loadData( artvert_image.getPixels(), artvert_image.getWidth(), artvert_image.getHeight(), gl_type );
			}
			else
			{
				fprintf(stderr, "couldn't load artvert image '%s', using fallback instead\n", artvert_image_file.c_str() );
				ofImage* fallback_image = fallback();
				GLuint gl_type;
				if ( fallback_image->type == OF_IMAGE_COLOR )
					gl_type = GL_RGB;
				else if ( fallback_image->type == OF_IMAGE_COLOR_ALPHA )
					gl_type = GL_RGBA;
				else
					gl_type = GL_LUMINANCE;
				texture.allocate( fallback_image->getWidth(), fallback_image->getHeight(), gl_type );
				texture.loadData( fallback_image->getPixels(), fallback_image->getWidth(), fallback_image->getHeight(), gl_type );
			}
		}
	}
	else if ( type == AV_3DS )
	{
		model_loader.loadModel(artvert_3ds_file, 30);
		model_loader.setRotation(0, 90, 1, 0, 0);
		model_loader.setRotation(1, 180, 0, 0, 1);
		model_loader.setScale(0.9, 0.9, 0.9);
		model_loader.setPosition(ofGetWidth()/2, ofGetHeight()/2, 0);
	}
	
	printf("artvert '%s': activated\n", getDescription().c_str() );
	active = true;
}

void Artvert::deactivate()
{
	printf("artvert '%s': deactivate() called\n", getDescription().c_str() );
	if ( !active )
		return; 
	if ( type == AV_MOVIE && avi_capture != NULL && avi_capture->width != 0 )
	{
		avi_capture->setVolume(0);
		avi_capture->stop();
	}
	else if ( type == AV_3DS )
	{
		// seems to be nothing to do
	}
	texture.clear();
	active = false;
}

ofImage* Artvert::fallback()
{
	if ( which_fallback_image == -1 )
		which_fallback_image = ofRandom( 0, 0.9999f * fallback_artvert_images.size() );
	return fallback_artvert_images[which_fallback_image];
}	


void Artvert::drawArtvert( float fade, const vector<float>& corners, bool for_preview )
{
	ofTexture* the_texture;
	if ( type == AV_MOVIE || type == AV_IMAGE )
	{
		if ( type == AV_MOVIE )
		{
			if ( !avi_capture || avi_capture->width == 0 )
				the_texture = &fallback()->getTextureReference();
			else
			{
				avi_capture->update();
				texture.loadData( avi_capture->getPixels(), avi_capture->width, avi_capture->height, GL_RGB );
				the_texture = &texture;
			}
		}
		else if ( type == AV_IMAGE )
		{
			the_texture = &texture;
		}
		else if ( type == AV_3DS )
		{
			glShadeModel (GL_SMOOTH);
			glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
			glEnable (GL_COLOR_MATERIAL);
		}
		
		glHint(GL_POLYGON_SMOOTH, GL_NICEST);
		glEnable(GL_POLYGON_SMOOTH);
		the_texture->bind();
		ofPoint texcoord, vec;
		glBegin(GL_QUADS);
		glColor4f(1.0, 1.0, 1.0, fade);
		texcoord = the_texture->getCoordFromPercent( 0, 0 );
		vec.set( corners[0], corners[1] );
		glTexCoord2f( texcoord.x, texcoord.y );
		glVertex3f( vec.x, vec.y, 0 );
		texcoord = the_texture->getCoordFromPercent( 1, 0 );
		vec.set( corners[2], corners[3] );
		glTexCoord2f( texcoord.x, texcoord.y );
		glVertex3f( vec.x, vec.y, 0 );
		texcoord = the_texture->getCoordFromPercent( 1, 1 );
		vec.set( corners[4], corners[5] );
		glTexCoord2f( texcoord.x, texcoord.y );
		glVertex3f( vec.x, vec.y, 0 );
		texcoord = the_texture->getCoordFromPercent( 0, 1 );
		vec.set( corners[6], corners[7] );
		glTexCoord2f( texcoord.x, texcoord.y );
		glVertex3f( vec.x, vec.y, 0 );
		glEnd();
		the_texture->unbind();
	}
	else
	{
		if ( for_preview )
		{
			ofPushStyle();
			ofSetColor( 255, 255, 255, 255 );
			float posx = (corners[0]+corners[2])/2 - 50;
			float posy = (corners[1]+corners[5])/2;
			ofDrawBitmapString( "(.3ds model)", posx, posy );
			ofPopStyle();
		}
		else
		{
			glShadeModel (GL_SMOOTH);
			glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
			glEnable (GL_COLOR_MATERIAL);
			glEnable(GL_DEPTH_TEST);
			glPushMatrix();

			float posx = corners[6] - corners[0];
			float posy = corners[3] - corners[1];
			glTranslatef(posx,posy,0);
			
			glEnable( GL_LIGHTING );
			model_loader.draw();
			glDisable( GL_LIGHTING );
			glPopMatrix();
			glDisable(GL_DEPTH_TEST);
		}
	}		
		
}

void Artvert::setVolume( float volume )
{
	if ( type == AV_MOVIE && avi_capture != NULL && avi_capture->width != 0 )
	{
		int passed_volume = volume*255;
		avi_capture->setVolume( passed_volume );
	}
}

string Artvert::getDescription()
{
	return advert_name + " '" + title + "' [" + artist + "]";
}

void Artvert::loadArtvertsFromXml( ofxXmlSettings& data, vector<Artvert*>& results )
{
	string model_file = data.getValue( "model_filename", "models/default.bmp" );
	// can use either 'advert' or 'name' for the name of this model/advert
	string advert_name;
	if ( data.getNumTags("advert") != 0 )
		advert_name = data.getValue( "advert", "unknown advert" );
	else 
		advert_name = data.getValue( "name", "unknown advert" );
	int num_artverts = data.getNumTags( "artvert" );
	printf("   -ml: got advert, model file '%s', advert '%s', %i artverts\n", model_file.c_str(), advert_name.c_str(), num_artverts );
	for ( int j=0; j<num_artverts; j++ )
	{
		Artvert* a = new Artvert();
		a->setModelFile( model_file );
		a->setAdvertName( advert_name );
		data.pushTag("artvert", j );
		// can use either 'name' or 'title' here
		if ( data.getNumTags( "name" ) != 0 )
			a->title = data.getValue( "name", "untitled" );
		else
			a->title = data.getValue( "title", "untitled" );
		a->artist = data.getValue( "artist", "unknown artist" );
		if ( data.getNumTags("movie_filename") != 0 )
		{
			// load a movie
			a->setArtvertMovieFile( data.getValue("movie_filename", "artverts/artvertmovie1.mp4" ) );
		}
		else if ( data.getNumTags("threeds_filename") != 0 )
		{
			// load a 3ds file
			a->setArtvert3dsFile( data.getValue("threeds_filename", "artverts/artvert3d1.3ds" ) );
		}
		else
		{
			// load an image
			a->setArtvertImageFile( data.getValue( "image_filename", "artverts/artvert1.png" ) );
		}
		string artvert_file;
		if ( a->type == AV_MOVIE )
			artvert_file = a->getArtvertMovieFile() + " (movie)";
		else if ( a->type == AV_3DS )
			artvert_file = a->getArtvert3dsFile() + " (.3ds)";
		else
			artvert_file = a->getArtvertImageFile() + " (image)";
		printf("     %i: %s:%s:%s\n", j, a->title.c_str(), a->artist.c_str(), artvert_file.c_str() );
		
		results.push_back( a );
		data.popTag();
	}
}	


void Artvert::saveArtvertToXml( ofxXmlSettings& data, Artvert* a, bool save_model )
{
	if ( save_model )
	{
		data.addValue( "model_filename", fromOfDataPath( a->model_file ) );
		// can use either 'advert' or 'name' for the name of this model/advert
		data.addValue( "name", a->advert_name );
	}
	int index = data.addTag( "artvert" );
	data.pushTag( "artvert", index );
	data.addValue( "title", a->title );
	data.addValue( "artist", a->artist );
	if ( a->type == AV_MOVIE )
	{
		data.addValue( "movie_filename", fromOfDataPath( a->getArtvertMovieFile() ) );
	}
	else if ( a->type == AV_3DS )
	{
		data.addValue( "threeds_filename", fromOfDataPath( a->getArtvert3dsFile() ) );
	}
	else
	{
		data.addValue( "image_filename", fromOfDataPath( a->getArtvertImageFile() ) );
	}
	data.popTag();
}	

void Artvert::changeArtvertFile( string new_filename )
{
	bool was_active = active;
	// clear out the old, if any
	shutdown();
	if ( new_filename.size() > 4 && new_filename.substr( new_filename.size()-4, 4 ) == ".3ds" )
	{
		// it's a 3ds model
		setArtvert3dsFile( new_filename );
	}
	// use FReeImage to tell us what kind of file it is..
	else if ( FreeImage_GetFileType( ofToDataPath( new_filename ).c_str() ) == FIF_UNKNOWN )
	{
		// it's a movie
		setArtvertMovieFile( new_filename );
	}
	else
	{
		// it's an image
		setArtvertImageFile( new_filename );
	}
	if ( was_active )
		activate();
		
}


string Artvert::getArtvertFile()
{ 
	
	if ( type == AV_MOVIE ) 
		return getArtvertMovieFile(); 
	else if ( type == AV_IMAGE )
		return getArtvertImageFile(); 
	else
		return getArtvert3dsFile();
}


void ArtvertDrawer::draw( float x, float y, float w, float h )
{
	if ( artvert == NULL || !artvert->isActive() ) 
		return;

	vector<float> corners;
	corners.push_back( x );
	corners.push_back( y );
	corners.push_back( x+w );
	corners.push_back( y );
	corners.push_back( x+w );
	corners.push_back( y+h );
	corners.push_back( x );
	corners.push_back( y+h );
	artvert->drawArtvert( 1.0f, corners, /*preview*/true );
}

	
