/*
 Copyright 2010 Damian Stewart <damian@frey.co.nz>.
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

#pragma once

#include "ofxMatrix4x4.h"
#include <cv.h>
#include "../FProfiler/FTime.h"
#include "ofxSemaphore.h"
#include <map>

// prune
#define PRUNE_MAX_SIZE 32


using namespace std;

class MatrixTracker
{
public:

    MatrixTracker();
    ~MatrixTracker();

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

    /// set kalman dt
    void setKalmanDt( float dt ) { kalman_dt = dt; }
    /// track using kalman. matrix is 3x4 with translation in last column.
    /// frame indices are monotonically-increasing indices come from the camera frames.
    void addPoseKalman( CvMat* matrix, unsigned int at_frame_index );
    /// fetch interpolated pose using kalman filtering.
    bool getInterpolatedPoseKalman( CvMat* matrix, unsigned int for_frame_index );
    bool getInterpolatedPoseKalman( ofxMatrix4x4& matrix, unsigned int for_frame_index );

    /// user interface
    float getPositionSmoothing()        { return position_smoothing; }
    float getPositionZSmoothing()       { return position_smoothing_z; }
    float getRotationSmoothing()        { return rotation_smoothing; }
    int getFramesBackRaw()              { return num_frames_back_raw; }
    int getFramesBackReturned()         { return num_frames_back_returned; }
    void increasePositionSmoothing()  { position_smoothing    = min(1.0f,position_smoothing*1.05f); }
    void increasePositionZSmoothing() { position_smoothing_z  = min(1.0f,position_smoothing_z*1.05f); }
    void increaseRotationSmoothing()  { rotation_smoothing    = min(1.0f,rotation_smoothing*1.05f); }
    void decreasePositionSmoothing()  { position_smoothing   /= 1.05f; }
    void decreasePositionZSmoothing() { position_smoothing_z /= 1.05f; }
    void decreaseRotationSmoothing()  { rotation_smoothing   /= 1.05f; }
    void increaseFramesBackRaw()      { num_frames_back_raw      = min(num_frames_back_raw+1, PRUNE_MAX_SIZE ); }
    void increaseFramesBackReturned() { num_frames_back_returned = min(num_frames_back_returned+1, PRUNE_MAX_SIZE ); }
    void decreaseFramesBackRaw()      { num_frames_back_raw      = max(num_frames_back_raw-1, 1 ); }
    void decreaseFramesBackReturned() { num_frames_back_returned = max(num_frames_back_returned-1, 1 ); }

private:

    void lock() { poses_lock.wait(); };
    void unlock() { poses_lock.signal(); };

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


    // kalman
    void createKalman();
    bool hasMeasurementForTime( unsigned int frame_index );
    typedef map<unsigned int, Pose> PoseMapKalman;
    PoseMapKalman kalman_found_poses;
    Pose predicted_pose_kalman;
    int kalman_curr_frame_index;
    float kalman_dt;
    CvKalman* kalman;
    CvRandState kalman_rng;
    CvMat* x_k; // predicted state
    CvMat* w_k; // process noise
    CvMat* z_k; // measurements


    /// estimate a pose by combining velocities for the last few frames
    /// poses is Pose[num_frames_back], times is FTime[num_frames_back]
    /// final_pos and final_rot are storage for output.
    void estimateNewPose( const Pose** poses, const FTime** times, int num_frames_back, const FTime& for_time,
                                    ofxVec3f& final_pos, ofxVec4f& final_rot );


    ofxVec3f prev_returned_translation;
    ofxQuaternion prev_returned_rotation;

    void make4x4MatrixFromQuatTrans( const ofxQuaternion& rot, const ofxVec3f& trans, ofxMatrix4x4& output );
    void smoothAndMakeMatrix( const ofxQuaternion& rot, const ofxVec3f& trans, const FTime& for_time, ofxMatrix4x4& output );

    ofxSemaphore poses_lock;


    float rotation_smoothing;
    float position_smoothing;
    float position_smoothing_z;
    int num_frames_back_returned;
    int num_frames_back_raw;


};


