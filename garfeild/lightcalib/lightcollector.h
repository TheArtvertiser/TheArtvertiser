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


#ifndef _LIGHTCOLLECTOR_H
#define _LIGHTCOLLECTOR_H

#include <cv.h>

class LightMap;

/*!
 * \ingroup photocalib
 * Collects lighting clues. This class apply an homography to a planar mesh and
 * average colors within triangles. The LightMap class can use these measures
 * to estimate a radiance map.
 */
class LightCollector {
public:
	LightCollector();
	LightCollector(const LightCollector &lc);
	~LightCollector();
	const LightCollector &operator=(LightCollector &lc);
	void copy(const LightCollector &lc);

	//! Generate a regular grid given its 4 corners.
	bool genGrid(float corners[4][2], int nx, int ny);

	void averageImage(IplImage *im, CvMat *_homography);

	int serializeSize();
	void serialize(char *buffer);
	void load(const char *buffer, int size);
	void invalidate();

	// for debugging
	void drawGrid(IplImage *im, CvMat *_homography);
	void drawAvg(IplImage *im);

	CvMat *worldRT;

	bool cmpWithRef(const LightCollector &ref, float *val, const float *scale=0, const float *shift=0);

	//friend LightMap;
//private:
	float *avg;
	CvMat *vertices, *transformed;
	int *triangles;
	int nbTri;
	int nx, ny;
	int avgChannels;
	int nbPix;
};

#endif
