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


#include "image_classifier.h"

image_classifier::image_classifier(LEARNPROGRESSION _LearnProgress)
{
  LearnProgression=_LearnProgress;
}


image_classifier::image_classifier(int _image_width, int _image_height, int _class_number,
                                   LEARNPROGRESSION _LearnProgress)
{
  LearnProgression=_LearnProgress;

  image_width  = _image_width;
  image_height = _image_height;
  class_number = _class_number;
}

