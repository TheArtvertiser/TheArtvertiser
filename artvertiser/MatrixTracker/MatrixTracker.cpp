#include "MatrixTracker.h"

#include "ofxMatrix4x4.h"

#include <stdio.h>

// prune
static const int PRUNE_MAX_SIZE = 8;

/*static const int NUM_FRAMES_BACK_RAW = 5;
static const int NUM_FRAMES_BACK_RETURNED = 20;*/


MatrixTracker::MatrixTracker()
{
    rotation_smoothing   = 0.95f;
    position_smoothing   = 0.7f;
    position_smoothing_z = 0.7f;

    // num_frames_back_* must be < PRUNE_MAX_SIZE
    num_frames_back_raw      = 5;
    num_frames_back_returned = 3;

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
        printf("t is %f: before %f requested %f after %f\n", t, prev_time.ToSeconds(), for_time.ToSeconds(), next_time.ToSeconds() );
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

