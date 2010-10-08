#include "testApp.h"

testApp::testApp( int _argc, char** _argv )
{
	argc = _argc;
	argv = _argv;
}

//--------------------------------------------------------------
void testApp::setup(){

	ofSetLogLevel( OF_LOG_VERBOSE );
	ofSetBackgroundAuto(false);
	artvertiser.setup( argc, argv );
}

//--------------------------------------------------------------
void testApp::update(){

	artvertiser.update();
}

//--------------------------------------------------------------
void testApp::draw(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
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

}

