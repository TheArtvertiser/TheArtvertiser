#pragma once

#include "ofxMatrix4x4.h"
#include <cv.h>
#include "../FProfiler/FTime.h"
#include <map>

using namespace std;

class MatrixTracker
{
public:

    /// track. matrix is 3x4 with translation in last column
    void addPose( CvMat* matrix, const FTime& timestamp );
    /// track. rotation_matrix is 3x3 rot matrix, trans is translation
    void addPose( const ofxMatrix4x4& rotation_matrix, const ofxVec3f& trans, const FTime& timestamp );

    /// Put an interpolated pose for the given timestamp into interpolated_pose, as a 3x4 matrix
    /// with translation in the last column. Return false if a pose couldn't be calculated.
    bool getInterpolatedPose( ofxMatrix4x4& interpolated_pose, const FTime& for_time );
    /// Put an interpolated pose for the given timestamp into interpolated_pose, as a 3x4 matrix
    /// with translation in the last column. Return false if a pose couldn't be calculated.
    bool getInterpolatedPose( CvMat* matrix, const FTime& for_time );

private:

    ofxVec3f translation;
    ofxQuaternion rotation;

    class Pose {
    public:
        Pose() {};
        Pose( const ofxVec3f& trans, const ofxQuaternion& rot ): translation(trans), rotation(rot) {}
        ofxVec3f translation;
        ofxQuaternion rotation;
    };

    // map allows us to do lower_bound and upper_bound
    typedef map<FTime, Pose> PoseMap;
    PoseMap found_poses;


    void make4x4MatrixFromQuatTrans( const ofxQuaternion& rot, const ofxVec3f trans, ofxMatrix4x4& output );

};


