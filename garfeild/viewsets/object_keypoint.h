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

#ifndef OBJECT_KEYPOINT_H
#define OBJECT_KEYPOINT_H

/*! A keypoint on a model
 * \ingroup viewsets
*/
class object_keypoint
{
public:
  double M[3];
  float scale;
  int class_index;

  void dump() { printf( " M %7.3f %7.3f %7.3f  scale %7.3f  class index %i\n",
                       M[0], M[1], M[2], scale, class_index ); }
};

#endif // OBJECT_KEYPOINT_H
