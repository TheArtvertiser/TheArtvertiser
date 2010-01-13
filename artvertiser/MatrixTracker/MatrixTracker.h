#pragma once

#include "ofxMatrix4x4.h"
#include <cv.h>

class MatrixTracker
{
public:

    /// track. matrix is 3x4 with translation in last column
    void track( CvMat* matrix );
    /// track. rotation_matrix is 3x3 rot matrix, trans is translation
    void track( const ofxMatrix4x4& rotation_matrix, const ofxVec3f& trans );

    //ofxMatrix4x4& getInterpolatedPosition();
    /// return interpolated position as 3x4 matrix with translation is the last column
    void getInterpolatedPose( ofxMatrix4x4& interpolated_pose );

private:

    ofxVec3f translation;
    ofxQuaternion rotation;


};


