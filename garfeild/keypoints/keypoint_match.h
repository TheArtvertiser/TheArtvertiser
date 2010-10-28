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

#ifndef KEYPOINT_MATCH_H
#define KEYPOINT_MATCH_H

#include "keypoint.h"

/*!
  \ingroup keypoints 
  \brief   A match between two \ref keypoint s.
  Contains a pointer on each keypoint, a score, and \em outlier boolean field.
*/

class keypoint_match
{
public:
  keypoint_match(void) { }

  keypoint * p1, * p2;
  float score;
  bool outlier;
};

#endif // KEYPOINT_MATCH_H
