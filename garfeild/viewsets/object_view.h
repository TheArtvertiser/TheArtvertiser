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


#ifndef OBJECT_VIEW_H
#define OBJECT_VIEW_H

#include <cv.h>

#include <keypoints/keypoint.h>
#include <keypoints/yape.h>
#include <viewsets/image_object_point_match.h>
#include <keypoints/keypoint.h>

/*! an object view contains 3 pyramids for image and gradient
  \ingroup viewsets
*/

class ofxBarrier;

class object_view
{
public:

  // Constructor for training stage:
  object_view(PyrImage * image);

  // Constructor for recognition stage (alloc memory once):
  object_view(int width, int height, int nbLev);

  ~object_view();

  void build_from_image_0(int kernelSize = 3);
  void build(IplImage *im, int kernelSize = -1);
  void comp_gradient();
  void comp_gradient_mt();
  static void* comp_gradient_thread_func(void* _data);

  PyrImage image;
  PyrImage gradX;
  PyrImage gradY;

  float affine_projection[6];
  int u0, v0;
  float alpha, scale;

  class CompGradientThreadData;
  vector<CompGradientThreadData*> comp_gradient_thread_data;
  ofxBarrier* shared_barrier;

};

#endif // OBJECT_VIEW_H
