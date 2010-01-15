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
    found_poses[timestamp] = Pose( new_translation, new_rotation );

    // limit size by trimming oldies
    if ( found_poses.size() > PRUNE_MAX_SIZE )
    {
        //delete (*found_poses.begin()).second;
        found_poses.erase( found_poses.begin() );
        assert( found_poses.size() == PRUNE_MAX_SIZE );
    }

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
        make4x4MatrixFromQuatTrans( (*prev_pose).second.rotation, (*prev_pose).second.translation, interpolated_pose );
        result = true;
    }
    else if ( next_pose == found_poses.end() )
    {
        // we only have prev : must estimate forwrads from known data
        //printf("future %f: after last (%f), forwards estimate not implemented yet\n", for_time.ToSeconds(), (--prev_pose)!=found_poses.begin()?(*prev_pose).first.ToSeconds():0.0f );
        //interpolated_pose.makeIdentityMatrix();
        // just use the last one
        make4x4MatrixFromQuatTrans( (*found_poses.rbegin()).second.rotation, (*found_poses.rbegin()).second.translation, interpolated_pose );
        result = true;
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
        make4x4MatrixFromQuatTrans( lerp_rot, lerp_trans, interpolated_pose );

        result = true;
    }

    return result;
}



//const ofxMatrix4x4&
void MatrixTracker::make4x4MatrixFromQuatTrans( const ofxQuaternion& rot, const ofxVec3f trans, ofxMatrix4x4& output )
{
    // fetch quaternion as matrix
    rot.get( output );
    // now write translation into last column
    output( 0, 3 ) = trans.x;
    output( 1, 3 ) = trans.y;
    output( 2, 3 ) = trans.z;
}
