#include "testApp.h"

testApp::testApp( int _argc, char** _argv )
{
	argc = _argc;
	argv = _argv;
}

//--------------------------------------------------------------
void testApp::setup(){

	ofSetWindowTitle(string("The Artvertiser ")+ARTVERTISER_VERSION);
	//ofSetLogLevel( OF_LOG_VERBOSE );
	ofSetBackgroundAuto(false);
	artvertiser.setup( argc, argv );
}

//--------------------------------------------------------------
void testApp::update(){

	artvertiser.update();
}

//--------------------------------------------------------------
void testApp::draw(){

	artvertiser.draw();

}

void testApp::exit()
{
	artvertiser.exitHandler();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	artvertiser.keyPressed( key );
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

	artvertiser.keyReleased( key );
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

	artvertiser.mouseMoved( x,y );

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

	artvertiser.mouseDragged( x, y, button );
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	artvertiser.mousePressed( x, y, button );

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	artvertiser.mouseReleased( x, y, button );

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	// ensure 4:3 ratio
	float aspect = float(w)/float(h);
	static const float desired_aspect = (4.0f/3.0f);
	if ( (aspect - desired_aspect) > 0.05f )
	{
		// less wide
		ofSetWindowShape( float(h)*desired_aspect, h );
	}
	else if ( (aspect-desired_aspect) < -0.05f )
	{
		// less high
		ofSetWindowShape( w, w/desired_aspect );
	}
	else
		artvertiser.windowResized( w, h );
}

