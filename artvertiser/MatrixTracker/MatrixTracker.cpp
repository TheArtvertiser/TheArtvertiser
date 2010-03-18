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

#include "MatrixTracker.h"

#include "ofxMatrix4x4.h"

#include <stdio.h>


/*static const int NUM_FRAMES_BACK_RAW = 5;
static const int NUM_FRAMES_BACK_RETURNED = 20;*/


MatrixTracker::MatrixTracker()
: kalman(0)
{
    rotation_smoothing   = 0.5f;
    position_smoothing   = 0.147f;
    position_smoothing_z = 0.7f;

    // num_frames_back_* must be < PRUNE_MAX_SIZE
    num_frames_back_raw      = 2;
    num_frames_back_returned = 10;

    kalman_dt = 0.05f; // 20 fps kalman default

}

MatrixTracker::~MatrixTracker()
{
    lock();
    if ( kalman )
    {
        cvReleaseKalman( &kalman );
        cvReleaseMat( &x_k );
        cvReleaseMat( &w_k );
        cvReleaseMat( &z_k );
    }
    unlock();
}




void MatrixTracker::addPose( CvMat* matrix, const FTime& timestamp )
{
    assert( matrix->width == 4 && matrix->height == 3 );

    // construct 4x4 rot matrix from 3x3 in top left of matrix,
    // filling in last row/col with 0,0,0,1
    ofxMatrix4x4 rot;
    for ( int i=0; i<4; i++ ) // y
        for ( int j=0; j<4; j++ ) // x
            rot._mat[i][j] = (i==3||j==3)?0:cvGet2D(matrix, i, j).val[0];
    rot._mat[3][3] = 1;

    ofxVec3f trans( cvGet2D(matrix, 0/*y*/, 3/*x*/ ).val[0], cvGet2D(matrix, 1, 3 ).val[0], cvGet2D( matrix, 2, 3 ).val[0] );

    addPose( rot, trans, timestamp );

}


void MatrixTracker::addPose( const ofxMatrix4x4& rot, const ofxVec3f& new_translation, const FTime& timestamp )
{
  //  printf("adding pose for timestamp %f\n", timestamp.ToSeconds() );

    // convert 4x4 rot matrix to quaternion
    ofxQuaternion new_rotation;
    new_rotation.set( rot );

    lock();

    found_poses[timestamp] = Pose( new_translation, new_rotation );

    // limit size by trimming oldies
    if ( found_poses.size() > PRUNE_MAX_SIZE )
    {
        //delete (*found_poses.begin()).second;
        found_poses.erase( found_poses.begin() );
        assert( found_poses.size() == PRUNE_MAX_SIZE );
    }

    unlock();

}

void MatrixTracker::addPoseKalman( CvMat* matrix, unsigned int at_frame_index )
{
    assert( matrix->width == 4 && matrix->height == 3 );

    // construct 4x4 rot matrix from 3x3 in top left of matrix,
    ofxMatrix4x4 rot;
    for ( int i=0; i<4; i++ ) // y
        for ( int j=0; j<4; j++ ) // x
            // fill; for last row/col use 0,0,0,0
            rot._mat[i][j] = (i==3||j==3)?0:cvGet2D(matrix, i, j).val[0];
    // now set bottom-right corner to 1
    rot._mat[3][3] = 1;

    ofxVec3f trans( cvGet2D(matrix, 0/*y*/, 3/*x*/ ).val[0], cvGet2D(matrix, 1, 3 ).val[0], cvGet2D( matrix, 2, 3 ).val[0] );

    // store in kalman map
    ofxQuaternion rot_quat;
    rot_quat.set( rot );

    lock();
    kalman_found_poses[ at_frame_index ] = Pose( trans, rot_quat );
    // limit size by trimming oldies
    if ( kalman_found_poses.size() > PRUNE_MAX_SIZE )
    {
        //delete (*found_poses.begin()).second;
        kalman_found_poses.erase( kalman_found_poses.begin() );
        assert( kalman_found_poses.size() == PRUNE_MAX_SIZE );
    }

    printf("added kalman pose at frame %i\n", at_frame_index );

    unlock();
}

bool MatrixTracker::getInterpolatedPoseKalman( CvMat* matrix, unsigned int for_frame_index )
{
    ofxMatrix4x4 interpolated_pose;
    bool res = getInterpolatedPoseKalman( interpolated_pose, for_frame_index );
    for ( int i=0; i<3; i++ )
        for ( int j=0; j<4; j++ )
            cvmSet( matrix, i, j, interpolated_pose( i, j ) );
    return res;
}

bool MatrixTracker::getInterpolatedPoseKalman( ofxMatrix4x4& interpolated_pose, unsigned int for_frame_index )
{
    lock();

    printf("kalman pose for frame %i requested\n", for_frame_index );

    if ( kalman == 0 )
        createKalman();

    // step through frames until we reach for_frame_index, updating kalman
    while ( kalman_curr_frame_index != for_frame_index /* use != not < in case it wraps */  )
    {
        // predict the new state
        const CvMat* y_k = cvKalmanPredict( kalman ); // predicted new state
        printf("kalman predicts:\n ");
        for ( int i=0; i<y_k->rows; i++ )
            printf("%7.3f ", cvmGet( y_k, i, 0 ) );
        printf("\n");

        // read state back
        predicted_pose_kalman.translation.set(cvmGet( y_k, 0, 0 ),
                                              cvmGet( y_k, 1, 0 ),
                                              cvmGet( y_k, 2, 0 )
                                              );
        ofxVec4f kalman_predicted_rot(        cvmGet( y_k, 3, 0 ),
                                              cvmGet( y_k, 4, 0 ),
                                              cvmGet( y_k, 5, 0 ),
                                              cvmGet( y_k, 6, 0 )
                                              );

        kalman_predicted_rot.normalize();
        predicted_pose_kalman.rotation = kalman_predicted_rot;
        printf("got rotation quaternion %7.3f %7.3f %7.3f %7.3f\n",
                kalman_predicted_rot.x,
                kalman_predicted_rot.y,
                kalman_predicted_rot.z,
                kalman_predicted_rot.w
               );

        // new measurement?
        if ( hasMeasurementForTime( kalman_curr_frame_index ) )
        {
            // copy measurement to kalman_measurement matrix (z_k)
            const Pose& measurement = kalman_found_poses[kalman_curr_frame_index];
            // store translation
            cvmSet( z_k, 0, 0, measurement.translation.x );
            cvmSet( z_k, 1, 0, measurement.translation.y );
            cvmSet( z_k, 2, 0, measurement.translation.z );
            // and rotation
            cvmSet( z_k, 3, 0, measurement.rotation.asVec4().x );
            cvmSet( z_k, 4, 0, measurement.rotation.asVec4().y );
            cvmSet( z_k, 5, 0, measurement.rotation.asVec4().z );
            cvmSet( z_k, 6, 0, measurement.rotation.asVec4().w );
            /*predicted_pose_kalman.rotation = measurement.rotation * (1.0f-rotation_smoothing)
                + predicted_pose_kalman.rotation * rotation_smoothing;*/
            // multiply measurements by measurement matrix
            // z_k = x_k*kalman->measurement_matrix + z_k
            //cvMatMulAdd( kalman->measurement_matrix, x_k, z_k, z_k );
            //cvMatMul( kalman->measurement_matrix, z_k, z_k );

            // correct the prediction against our measurements
            cvKalmanCorrect( kalman, z_k );
        }
        else
        {
            // update process noise (w_k)
            cvRand( &kalman_rng, w_k );
            // multiply by process noise matrix
            cvMatMul( kalman->process_noise_cov, w_k, w_k );
            // update by pushing last predicted state through transition matrix, adding process noise
            cvMatMulAdd( kalman->transition_matrix, y_k, w_k, x_k );
            // read off values for z
            printf("pushing prediction through transition matrix gives us:\n ");
            for ( int i=0; i<3+4; i++ )
            {
                printf("%7.3f ", cvmGet( x_k, i, 0 ) );
                cvmSet( z_k, i, 0, cvmGet( x_k, i, 0 ) );
            }
            printf("\n");
            // normalise the rotation quaternion, to be sure
            ofxVec4f rot_quat(cvmGet(x_k, 3, 0),
                              cvmGet(x_k, 4, 0),
                              cvmGet(x_k, 5, 0),
                              cvmGet(x_k, 6, 0));
            rot_quat.normalize();
            // put back
            cvmSet( z_k, 3, 0, rot_quat.x );
            cvmSet( z_k, 4, 0, rot_quat.y );
            cvmSet( z_k, 5, 0, rot_quat.z );
            cvmSet( z_k, 6, 0, rot_quat.w );
            // update kalman
            cvKalmanCorrect( kalman, z_k );
        }


        /*// copy pre to post
        memcpy( x_k->data.fl, kalman->state_post->data.fl,
               sizeof(float)*kalman->state_pre->cols*kalman->state_pre->rows );*/


        // step time forward
        kalman_curr_frame_index++;
    }

    // return
    make4x4MatrixFromQuatTrans(predicted_pose_kalman.rotation,
                               predicted_pose_kalman.translation,
                               interpolated_pose );
    unlock();
}

bool MatrixTracker::hasMeasurementForTime( unsigned int frame_index )
{
    return (kalman_found_poses.find(frame_index)!= kalman_found_poses.end());
}

void MatrixTracker::createKalman()
{
    assert( kalman == 0 );

    // construct kalman struct
    int num_dynamic_params = 3+3+4+4; // pos+vel+rot+vr
    int num_measured_params = 3+4;  // pos, rot
    kalman = cvCreateKalman( num_dynamic_params, num_measured_params );

    // setup transition matrix
    const float dt = kalman_dt;
    const float transition_matrix[] = { /*px*/ 1,  0,  0,  0,  0,  0,  0, dt,  0,  0,  0,  0,  0,  0, // px = px + dt*vx;
                                        /*py*/ 0,  1,  0,  0,  0,  0,  0,  0, dt,  0,  0,  0,  0,  0, // py = py + dt*vy;
                                        /*pz*/ 0,  0,  1,  0,  0,  0,  0,  0,  0, dt,  0,  0,  0,  0, // pz = pz + dt*vz;
                                        /*rot*/0,  0,  0,  1,  0,  0,  0,  0,  0,  0, dt,  0,  0,  0, // rx = rx + dt*vrx
                                        /*rot*/0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0, dt,  0,  0, // ...
                                        /*rot*/0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0, dt,  0,
                                        /*rot*/0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0, dt,
                                        /*vx*/ 0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
                                        /*vy*/ 0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,
                                        /*vz*/ 0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,
                                        /*vr */0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,
                                        /*vr */0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,
                                        /*vr */0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,
                                        /*vr */0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1
                                         };
    memcpy( kalman->transition_matrix->data.fl, transition_matrix, sizeof(transition_matrix) );

    // setup other matrices
    w_k = cvCreateMat( num_dynamic_params,  1, CV_32FC1 );
    z_k = cvCreateMat( num_measured_params, 1, CV_32FC1 );
    x_k = cvCreateMat( num_dynamic_params,  1, CV_32FC1 );

    // initialise remaining kalman struct

    // measurement matrix is num_measured_params rows x num_dynamic_params cols
    cvSetIdentity( kalman->measurement_matrix,      cvRealScalar(1) );

    // dunno what this is
    cvSetIdentity( kalman->error_cov_pre,           cvRealScalar(1) );

    // how noisy are our measurements?
    cvSetIdentity( kalman->process_noise_cov );
    cvSetIdentity( kalman->measurement_noise_cov );
    // position is quite noisy
    for ( int i=0; i<3; i++ )
    {
        cvmSet( kalman->process_noise_cov,     i, i, 1e-2 );
        cvmSet( kalman->measurement_noise_cov, i, i, 1 );
    }
    // rotation is not so noisy
    for ( int i=3; i<7; i++ )
    {
        cvmSet( kalman->process_noise_cov,     i, i, 1e-3 );
        cvmSet( kalman->measurement_noise_cov, i, i, 1e-1 );
    }

    // choose random initial state
    cvRandInit( &kalman_rng, 0, 1, -1, CV_RAND_UNI );

    /*cvRandSetRange( &kalman_rng, 0, 0.1, 0  );
    kalman_rng.disttype = CV_RAND_NORMAL;*/
    cvRand( &kalman_rng, x_k );
    //cvRand( &kalman_rng, kalman->state_post );
    cvZero( x_k );
    cvZero( kalman->state_post );
    cvmSet( x_k, 7, 0, 1 );
    cvmSet( kalman->state_post, 7, 0, 1 );

}

/*
    // smooth
    // motivation: translation should be allowed to change faster than orientation
    translation = 0.3f*translation + 0.7f*new_translation;
    // so we allow the rotation to lag on a bit
    rotation = 0.125f*rotation.asVec4() + 0.875f*new_rotation.asVec4();
    //so = new_so;


  //  printf("so: %10f %10f %10f %10f\n", so.asVec4().x, so.asVec4().y, so.asVec4().z, so.asVec4().w );
  //  printf("scale: %10f %10f %10f\n", scale.x, scale.y, scale.z );

    printf("translation now %10f %10f %10f\n", translation.x, translation.y, translation.z );
    ofxVec4f r = rotation.asVec4();
    printf("rotation now  %10f %10f %10f %10f\n", r.x, r.y, r.z, r.w );

    //printf("translation: decomposed %10f %10f %10f, source %10f %10f %10f\n",
    //       new_translation.x, new_translation.y, new_translation.z,
    //       matrix(3,0), matrix(3,1), matrix(3,2) );

    */




bool MatrixTracker::getInterpolatedPose( CvMat* matrix, const FTime& for_time )
{
    ofxMatrix4x4 interpolated_pose;
    bool res = getInterpolatedPose( interpolated_pose, for_time );
    for ( int i=0; i<3; i++ )
        for ( int j=0; j<4; j++ )
            cvmSet( matrix, i, j, interpolated_pose( i, j ) );
    return res;
}


bool MatrixTracker::getInterpolatedPose( ofxMatrix4x4& interpolated_pose, const FTime& for_time )
{

    // so then
    // where are we?

    lock();

    PoseMap::iterator prev_pose = found_poses.lower_bound( for_time );
    PoseMap::iterator next_pose = found_poses.upper_bound( for_time );
    // we must step forward
    /*if ( next_pose != found_poses.end() )
        ++next_pose;*/
    //if ( prev_pose !=   )

    bool result = false;

    if ( found_poses.size() == 0 )
    {
        // nothing there
        //printf("empty\n");
        interpolated_pose.makeIdentityMatrix();
        result = false;
    }
    else if ( for_time == (*prev_pose).first )
    {
        // exactly on first
        //printf("exactly on prev\n");
        smoothAndMakeMatrix( (*prev_pose).second.rotation, (*prev_pose).second.translation,
                            for_time, interpolated_pose );
        result = true;
    }
    else if ( next_pose == found_poses.end() )
    {
        // we only have prev : must estimate forwrads from known data
        //printf("future %f: after last (%f), forwards estimate: \n", for_time.ToSeconds(), (--prev_pose)!=found_poses.begin()?(*prev_pose).first.ToSeconds():0.0f );
        //interpolated_pose.makeIdentityMatrix();


        /*// just use the last one
        make4x4MatrixFromQuatTrans( (*found_poses.rbegin()).second.rotation, (*found_poses.rbegin()).second.translation, interpolated_pose );
        */
        --prev_pose;
        // walk back 3 frame
        const FTime* times[num_frames_back_raw];// = (*prev_pose).first;
        const Pose* poses[num_frames_back_raw];//  = (*prev_pose).second;
        int last_frame_back = num_frames_back_raw-1;
        bool failed = false;
        // collect previous poses: bail and set failed false if we don't have enough
        for ( int i=last_frame_back; i>=0 ;i-- )
        {
            if ( prev_pose==found_poses.begin() )
            {
                smoothAndMakeMatrix( (*found_poses.rbegin()).second.rotation,
                                    (*found_poses.rbegin()).second.translation,
                                    for_time, interpolated_pose );
                failed = true;
                break;
            }

            times[i] = &(*prev_pose).first;
            poses[i] = &(*prev_pose).second;
            --prev_pose;
        }
        /*
        i 2 == now
        i 1 == then
        i 0 == ages ago
        */

        if ( !failed )
        {
            // estimate from priors
            ofxVec4f final_rot;
            ofxVec3f final_pos;
            estimateNewPose( poses, times, num_frames_back_raw, for_time, final_pos, final_rot );

            //make4x4MatrixFromQuatTrans( final_rot_quat, final_pos, interpolated_pose );
            smoothAndMakeMatrix( final_rot, final_pos, for_time, interpolated_pose );
        }

        result = !failed;

    }
    else if ( prev_pose == found_poses.begin() )
    {
        // we don't have prev : must estimate backwards from known data
        //printf("past: before first, backwards estimate not implemented yet\n");
        interpolated_pose.makeIdentityMatrix();
        result = false;
    }
    else
    {
        // we have both: interpolate
        // pull prev back
        --prev_pose;

        // fetch endpoints
        const FTime& prev_time =    (*prev_pose).first;
        const Pose& prev_p =        (*prev_pose).second;
        const FTime& next_time =    (*next_pose).first;
        const Pose& next_p =        (*next_pose).second;

        // lerp
        ofxQuaternion lerp_rot;
        ofxVec3f lerp_trans;
        // calculate t percentage
        float t = (for_time.ToSeconds()-prev_time.ToSeconds())/(next_time.ToSeconds()-prev_time.ToSeconds());
        //printf("t is %f: before %f requested %f after %f\n", t, prev_time.ToSeconds(), for_time.ToSeconds(), next_time.ToSeconds() );
        lerp_rot.slerp( t, prev_p.rotation, next_p.rotation );
        lerp_trans = prev_p.translation + t*(next_p.translation-prev_p.translation);

        // store in output
        smoothAndMakeMatrix( lerp_rot, lerp_trans, for_time, interpolated_pose );

        result = true;
    }

    unlock();

    return result;
}



//const ofxMatrix4x4&
void MatrixTracker::make4x4MatrixFromQuatTrans( const ofxQuaternion& rot, const ofxVec3f& trans, ofxMatrix4x4& output )
{
    // fetch quaternion as matrix
    rot.get( output );
    // now write translation into last column
    output( 0, 3 ) = trans.x;
    output( 1, 3 ) = trans.y;
    output( 2, 3 ) = trans.z;
}

void MatrixTracker::smoothAndMakeMatrix( const ofxQuaternion& final_rot, const ofxVec3f& final_pos,
                                        const FTime& for_time, ofxMatrix4x4& output )
{

    // look up previous n poses in returned_poses
    const FTime* times_returned[num_frames_back_returned];
    const Pose*  poses_returned[num_frames_back_returned];
    int last_frame_back = num_frames_back_returned-1;
    bool failed = false;
    // collect previous poses: bail and set failed false if we don't have enough
    PoseMap::reverse_iterator it = returned_poses.rbegin();
    for ( int i=last_frame_back; i>=0; i-- )
    {
        if ( it==returned_poses.rend() )
        {
            // smoothAndMakeMatrix( (*found_poses.rbegin()).second.rotation, (*found_poses.rbegin()).second.translation, interpolated_pose );
            failed = true;
            break;
        }

        times_returned[i] = &(*it).first;
        poses_returned[i] = &(*it).second;

        ++it;
    }

    ofxVec3f output_pos;
    ofxVec4f output_rot;
    if ( failed )
    {
        output_pos = final_pos;
        output_rot = final_rot.asVec4();
    }
    else
    {
        ofxVec3f final_pos_returned;
        ofxVec4f final_rot_returned;
        estimateNewPose( poses_returned, times_returned, num_frames_back_returned,
                        for_time, final_pos_returned, final_rot_returned );

        output_rot = final_rot.asVec4()*(1.0f-rotation_smoothing) + final_rot_returned*rotation_smoothing;
        output_pos = final_pos*(1.0f-position_smoothing)          + final_pos_returned*position_smoothing;
        // different smoothing for z
        output_pos.z = final_pos.z*(1.0f-position_smoothing_z)    + final_pos_returned.z*position_smoothing_z;
    }

    // make matrix
    make4x4MatrixFromQuatTrans( output_rot, output_pos, output );

    // store
    returned_poses[for_time] = Pose( output_pos, output_rot );
    // limit size by trimming oldies
    if ( returned_poses.size() > PRUNE_MAX_SIZE )
    {
        //delete (*found_poses.begin()).second;
        returned_poses.erase( returned_poses.begin() );
        assert( returned_poses.size() == PRUNE_MAX_SIZE );
    }


/*
    // deal with z: it should change very slowly
    float prev_returned_translation_z;
    prev_returned_translation_z = final_pos.z*0.1f + prev_returned_translation.z*0.9f;

    // average with prev returned
    prev_returned_rotation    = final_rot*0.1f + prev_returned_rotation*0.9f;
    prev_returned_translation = final_pos*0.5f + prev_returned_translation*0.5f;

    // use z
    prev_returned_translation.z = prev_returned_translation_z;

    make4x4MatrixFromQuatTrans( prev_returned_rotation, prev_returned_translation, output );
    //make4x4MatrixFromQuatTrans( final_rot, final_pos, output );*/
}


/*void MatrixTracker::smoothAndMakeMatrix( const ofxQuaternion& final_rot, const ofxVec3f& final_pos, ofxMatrix4x4& output )
{
    // deal with z: it should change very slowly
    float prev_returned_translation_z;
    prev_returned_translation_z = final_pos.z*0.1f + prev_returned_translation.z*0.9f;

    // average with prev returned
    prev_returned_rotation    = final_rot*0.1f + prev_returned_rotation*0.9f;
    prev_returned_translation = final_pos*0.5f + prev_returned_translation*0.5f;

    // use z
    prev_returned_translation.z = prev_returned_translation_z;

    make4x4MatrixFromQuatTrans( prev_returned_rotation, prev_returned_translation, output );
    //make4x4MatrixFromQuatTrans( final_rot, final_pos, output );
}
*/

void MatrixTracker::estimateNewPose( const Pose** poses, const FTime** times, int num_frames_back, const FTime& for_time,
                                    ofxVec3f& final_pos, ofxVec4f& final_rot )
{
    // calculate velocity: use (/*9*values[2]+*/ 4*values[1] + values[0])/14.0:
    ofxVec3f pos_velocity;
    ofxVec4f rot_velocity;
    float total_rot_mul = 0;
    float total_pos_mul = 0;
    int last_frame_back = num_frames_back-1;
    for ( int i=0; i<last_frame_back - 1; i++ )
    {
        // square falloff : fresher = more important
        double abs_delta_time = for_time.ToSeconds() - times[i+1]->ToSeconds();
        //float pos_mul = (i+1)/**(i+1)*/;
        //float rot_mul = sqrtf(i+1);
        float pos_mul = 1.0f/(abs_delta_time*abs_delta_time);
        float rot_mul = 1.0f/abs_delta_time;
        double this_delta_time = times[i+1]->ToSeconds() - times[i]->ToSeconds();
        ofxVec3f this_pos_velocity = (poses[i+1]->translation       - poses[i]->translation)       / this_delta_time;
        ofxVec4f this_rot_velocity = (poses[i+1]->rotation.asVec4() - poses[i]->rotation.asVec4()) / this_delta_time;

        // accumulate
        pos_velocity += pos_mul*this_pos_velocity;
        rot_velocity += rot_mul*this_rot_velocity;
        total_rot_mul += rot_mul;
        total_pos_mul += pos_mul;
    }
    pos_velocity /= total_pos_mul;
    rot_velocity /= total_rot_mul;

    // so now: we have a position velocity + a rotation velocity as at times[2]
    double time_from_prev = for_time.ToSeconds() - times[last_frame_back]->ToSeconds();
    /*printf("  dt %f: pos vel %6.3f %6.3f %6.3f, rot vel %6.3f %6.3f %6.3f %6.3f\n",
           time_from_prev, pos_velocity.x, pos_velocity.y, pos_velocity.z,
           rot_velocity.x, rot_velocity.y, rot_velocity.z, rot_velocity.w );*/

    // so final pos/rot are
    final_pos = poses[last_frame_back]->translation + pos_velocity*time_from_prev;
    final_rot = poses[last_frame_back]->rotation.asVec4()    + rot_velocity*time_from_prev;
}

