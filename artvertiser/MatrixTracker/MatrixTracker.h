#pragma once

#include "ofxMatrix4x4.h"


class MatrixTracker
{
public:

    void track( const ofxMatrix4x4& matrix );


private:

    ofxVec3f translation;
    ofxQuaternion rotation;
    ofxVec3f scale;
    ofxQuaternion so;

};


