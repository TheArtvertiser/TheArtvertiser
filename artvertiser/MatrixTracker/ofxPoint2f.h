/*
 Copyright 2009 openFrameworks
 
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
/**
*  ofxPoint2f.h
*  by stefanix
*
* DEPRECATED!! use ofxVec2f.h for both points and vectors.
*/


#ifndef _OFX_POINT2f
#define _OFX_POINT2f

#if (_MSC_VER)
#pragma message("warning: ofxPoint2f is deprecated, use ofxVec2f.h for both points and vectors")
#else
#warning "ofxPoint2f is deprecated, use ofxVec2f.h for both points and vectors"
#endif


#include "ofxVec2f.h"

typedef ofxVec2f  ofxPoint2f;


#endif
