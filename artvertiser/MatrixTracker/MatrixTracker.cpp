#include "MatrixTracker.h"

#include "ofxMatrix4x4.h"

#include <stdio.h>

// prune
static const int PRUNE_MAX_SIZE = 8;

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
        smoothAndMakeMatrix( (*prev_pose).second.rotation, (*prev_pose).second.translation, interpolated_pose );
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
        static const int NUM_FRAMES_BACK = 5;
        static const int LAST_FRAME_BACK = NUM_FRAMES_BACK-1;
        const FTime* times[NUM_FRAMES_BACK];// = (*prev_pose).first;
        Pose*  poses[NUM_FRAMES_BACK];//  = (*prev_pose).second;
        bool failed = false;
        // collect previous poses: bail and set failed false if we don't have enough
        for ( int i=LAST_FRAME_BACK; i>=0 ;i-- )
        {
            if ( prev_pose==found_poses.begin() )
            {
                smoothAndMakeMatrix( (*found_poses.rbegin()).second.rotation, (*found_poses.rbegin()).second.translation, interpolated_pose );
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
            // calculate velocity: use (/*9*values[2]+*/ 4*values[1] + values[0])/14.0:
            ofxVec3f pos_velocity;
            ofxVec4f rot_velocity;
            float total_rot_mul = 0;
            float total_pos_mul = 0;
            for ( int i=0; i<LAST_FRAME_BACK - 1; i++ )
            {
                // square falloff : fresher = more important <-- todo: link to this_time_delta somehow?
                double abs_delta_time = for_time.ToSeconds() - times[i+1]->ToSeconds();
                //float pos_mul = (i+1)/**(i+1)*/;
                //float rot_mul = sqrtf(i+1);
                float pos_mul = 1.0f/(1.0f+abs_delta_time*abs_delta_time);
                float rot_mul = 1.0f/(1.0f+abs_delta_time);
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
            double time_from_prev = for_time.ToSeconds() - times[LAST_FRAME_BACK]->ToSeconds();
            /*printf("  dt %f: pos vel %6.3f %6.3f %6.3f, rot vel %6.3f %6.3f %6.3f %6.3f\n",
                   time_from_prev, pos_velocity.x, pos_velocity.y, pos_velocity.z,
                   rot_velocity.x, rot_velocity.y, rot_velocity.z, rot_velocity.w );*/

            // so final pos/rot are
            ofxVec3f final_pos = poses[LAST_FRAME_BACK]->translation + pos_velocity*time_from_prev;
            ofxVec4f final_rot = poses[LAST_FRAME_BACK]->rotation.asVec4()    + rot_velocity*time_from_prev;

            ofxQuaternion final_rot_quat( final_rot );

            smoothAndMakeMatrix( final_rot_quat, final_pos, interpolated_pose );
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
        smoothAndMakeMatrix( lerp_rot, lerp_trans, interpolated_pose );

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


void MatrixTracker::smoothAndMakeMatrix( const ofxQuaternion& final_rot, const ofxVec3f& final_pos, ofxMatrix4x4& output )
{
    // average with prev returned
    prev_returned_rotation = final_rot*0.3333f + prev_returned_rotation*0.6666f;
    prev_returned_translation = final_pos*0.6666f + prev_returned_translation*0.3333f;

    make4x4MatrixFromQuatTrans( prev_returned_rotation, prev_returned_translation, output );
    //make4x4MatrixFromQuatTrans( final_rot, final_pos, output );
}
