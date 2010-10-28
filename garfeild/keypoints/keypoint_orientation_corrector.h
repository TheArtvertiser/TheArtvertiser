/*
 Copyright 2005, 2006 Computer Vision Lab,
 Ecole Polytechnique Federale de Lausanne (EPFL), Switzerland.
 Modified by Damian Stewart <damian@frey.co.nz> 2009-2010;
 modifications Copyright 2009, 2010 Damian Stewart <damian@frey.co.nz>.

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


#ifndef KEYPOINT_ORIENTATION_CORRECTOR_H
#define KEYPOINT_ORIENTATION_CORRECTOR_H

#include <cv.h>


/*!\brief Normalize local patches against rotation
 * \ingroup keypoints
 */
class  keypoint_orientation_corrector
{
public:
   keypoint_orientation_corrector(int width, int height, int neighborhood_size, int nbLev);
   ~keypoint_orientation_corrector();

   void compute_gradient_images(IplImage * image, IplImage ** Ix, IplImage ** Iy);

   //! Returns the estimated orientation (actually the corresponding bucket index)
   int orientation_bucket_index(const IplImage * _Ix, const IplImage * _Iy, const int u, const int v);
   //! Optimized version.
   int optimized_orientation_bucket_index(const IplImage * _Ix, const IplImage * _Iy, const int u, const int v);

   //! returns the estimated orientation expressed in radians
   float estimate_orientation_in_radians(IplImage * image, int u, int v, IplImage * _Ix, IplImage * _Iy);

   //! Rotates the (u,v) neighborhood and returns the estimated orientation (actually the corresponding bucket index):
   int correct_orientation(IplImage * image, int u, int v, 
                           IplImage * rotated_neighborhood, 
                           IplImage * Ix, IplImage * Iy, int level);

   int correct_orientationf(IplImage * image, float u, float v, 
                            IplImage * rotated_neighborhood, 
                            float orientation_in_radians, int level);

   static bool subpixel_rotate;

   static const int ANGLE_QUANTUM = 20; // in degrees
   static const int ANGLE_BUCKET_NUMBER = 360 / ANGLE_QUANTUM;
private:
   void initialize_tables(void);
   void rotate_patch(IplImage * original_image, int u, int v, 
                                                  IplImage * rotated_patch,
                                                  int orientation_bucket_index, int level);
   void rotate_patchf(IplImage * original_image, float u, float v, 
                                                  IplImage * rotated_patch,
                                                  float angle);

   int width, height, neighborhood_size, actual_neighborhood_size;

   short * atan2_table;
   int * angle_buckets;
   IplImage * exp_weights;
  
   int ** orientation_lookup_tables;
   int nbLev;
};

#endif // KEYPOINT_ORIENTATION_CORRECTOR_H
