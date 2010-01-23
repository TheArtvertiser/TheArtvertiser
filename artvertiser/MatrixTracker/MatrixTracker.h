#pragma once

#include "ofxMatrix4x4.h"
#include <cv.h>
#include "../FProfiler/FTime.h"
#include "../FProfiler/FSemaphore.h"
#include <map>

using namespace std;

class MatrixTracker
{
public:
    MatrixTracker();

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

    void lock() { poses_lock.Wait(); };
    void unlock() { poses_lock.Signal(); };

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
    PoseMap returned_poses;


    /// estimate a pose by combining velocities for the last few frames
    /// poses is Pose[num_frames_back], times is FTime[num_frames_back]
    /// final_pos and final_rot are storage for output.
    void estimateNewPose( const Pose** poses, const FTime** times, int num_frames_back, const FTime& for_time,
                                    ofxVec3f& final_pos, ofxVec4f& final_rot );


    ofxVec3f prev_returned_translation;
    ofxQuaternion prev_returned_rotation;

    void make4x4MatrixFromQuatTrans( const ofxQuaternion& rot, const ofxVec3f& trans, ofxMatrix4x4& output );
    void smoothAndMakeMatrix( const ofxQuaternion& rot, const ofxVec3f& trans, const FTime& for_time, ofxMatrix4x4& output );

    FSemaphore poses_lock;


    float rotation_smoothing;
    float position_smoothing;
    float position_smoothing_z;
    int num_frames_back_returned;
    int num_frames_back_raw;


};


