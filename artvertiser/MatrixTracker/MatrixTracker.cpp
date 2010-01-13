#include "MatrixTracker.h"

#include "ofxMatrix4x4.h"

#include <stdio.h>

void MatrixTracker::track( CvMat* matrix )
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

    track( rot, trans );

}


void MatrixTracker::track( const ofxMatrix4x4& rot, const ofxVec3f& new_translation )
{

    // convert 4x4 rot matrix to quaternion
    ofxQuaternion new_rotation;
    new_rotation.set( rot );

    // smooth
    translation = (7.0f*translation + new_translation)/8.0;
    //scale = new_scale;
    rotation = (7.0f*rotation.asVec4() + new_rotation.asVec4())/8.0;
    //so = new_so;


/*    printf("so: %10f %10f %10f %10f\n", so.asVec4().x, so.asVec4().y, so.asVec4().z, so.asVec4().w );
    printf("scale: %10f %10f %10f\n", scale.x, scale.y, scale.z );
    */
    printf("translation now %10f %10f %10f\n", translation.x, translation.y, translation.z );
    ofxVec4f r = rotation.asVec4();
    printf("rotation now  %10f %10f %10f %10f\n", r.x, r.y, r.z, r.w );

    /*printf("translation: decomposed %10f %10f %10f, source %10f %10f %10f\n",
           new_translation.x, new_translation.y, new_translation.z,
           matrix(3,0), matrix(3,1), matrix(3,2) );*/

}


void MatrixTracker::getInterpolatedPose( ofxMatrix4x4& interpolated_pose )
{
    // fetch quaternion as matrix
    rotation.get( interpolated_pose );
    // now write translation into last column
    interpolated_pose( 0, 3 ) = translation.x;
    interpolated_pose( 1, 3 ) = translation.y;
    interpolated_pose( 2, 3 ) = translation.z;
}



//const ofxMatrix4x4&
