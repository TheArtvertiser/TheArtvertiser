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

#include "growmat.h"

CvGrowMat::CvGrowMat(int maxlines, int maxcols, int type)
{
	mat = cvCreateMat(maxlines, maxcols, type);
	cvSetZero(mat);
	cvGetSubRect(mat, this, cvRect(0,0,maxcols,maxlines));
}

CvGrowMat::~CvGrowMat()
{
	cvReleaseMat(&mat);
}

void CvGrowMat::resize(int lines, int cols)
{
	if (lines <= mat->rows && cols <= mat->cols) {
		cvGetSubRect(mat, this, cvRect(0,0,cols,lines));
		//this->rows = lines;
		//this->cols = cols;
	} else {
		int nl = (lines > mat->rows ? lines*2 : mat->rows);
		int nc = (cols > mat->cols ? cols*2 : mat->cols);
		CvMat *nm = cvCreateMat(nl, nc, mat->type);
		cvSetZero(nm);
		if (this->rows && this->cols) {
			CvMat sub;
			cvGetSubRect(nm, &sub, cvRect(0,0,this->cols, this->rows));
			cvCopy(this, &sub);
			cvGetSubRect(nm, this, cvRect(0,0,cols, lines));
		} else {
			cvGetSubRect(nm, this, cvRect(0,0,mat->cols, mat->rows));
			this->rows = lines;
			this->cols = cols;
		}
		cvReleaseMat(&mat);
		mat = nm;
	}
}

