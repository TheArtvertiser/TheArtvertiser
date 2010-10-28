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

#ifndef _MATVEC_H
#define _MATVEC_H

#include <iostream>
#include <math.h>
#include <cv.h>

struct Mat3x4;
struct Mat3x3;

struct Vec3 {
	double v[3];

	Vec3() {}
	Vec3(const double p[3]) { v[0]=p[0]; v[1]=p[1]; v[2]=p[2]; }
	Vec3(double x, double y, double z) { v[0]=x; v[1]=y; v[2]=z; }

	double &operator[](int i) { return v[i]; }
	const double &operator[](int i) const { return v[i]; }

	const Vec3 &operator = (const Vec3 &a) {
		v[0]=a[0]; v[1]=a[1]; v[2]=a[2]; return *this;
	}

	void mul(double scalar) {
		v[0] *= scalar;
		v[1] *= scalar;
		v[2] *= scalar;
	}

	double norm() const { return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]); }
	void normalize() { mul(1.0/norm()); }

	void setMul(const Mat3x4 &mat, const Vec3 &v);

	void setSub(const Vec3 &a, const Vec3 &b) {
		v[0] = a[0]-b[0]; v[1] = a[1]-b[1]; v[2] = a[2]-b[2];
	}
	void setAdd(const Vec3 &a, const Vec3 &b) {
		v[0] = a[0]+b[0]; v[1] = a[1]+b[1]; v[2] = a[2]+b[2];
	}
	void sub(const Vec3 &a) { v[0] -= a[0]; v[1]-=a[1]; v[2]-=a[2]; }
	void add(const Vec3 &a) { v[0] += a[0]; v[1]+=a[1]; v[2]+=a[2]; }

	void setCross(const Vec3 &a, const Vec3 &b) {
		v[0] = a[1] * b[2] - a[2] * b[1];
		v[1] = a[2] * b[0] - a[0] * b[2];
		v[2] = a[0] * b[1] - a[1] * b[0];
	}

	void set(double x, double y, double z) { v[0]=x; v[1]=y; v[2]=z; }
	void set(double *a) { v[0]=a[0]; v[1]=a[1]; v[2]=a[2]; }

	double dot(const Vec3 &a) {
		return a[0]*v[0] + a[1]*v[1] + a[2]*v[2];
	}
};

struct Mat3x4 {
	double m[3][4];

	void set(CvMat *m);
	void setIdentity();
	void setMul(const Mat3x3 &a, const Mat3x4 &b);
	void setMul(const Mat3x4 &a, const Mat3x4 &b);
	void mul(const Mat3x4 &a);
	void setInverseByTranspose(const Mat3x4 &m);
	void setRotate(const Vec3 &axis, double angle);
	void setTranslate(double x, double y, double z);
	void transform(const float src[4], float dst[4]);
	double det() { return  m[0][0]*m[1][1]*m[2][2] -m[0][0]*m[1][2]*m[2][1] -m[1][0]*m[0][1]*m[2][2]
			+m[1][0]*m[0][2]*m[2][1] +m[2][0]*m[0][1]*m[1][2]-m[2][0]*m[0][2]*m[1][1]; }
};

std::ostream& operator << (std::ostream& os, const Mat3x4 &cam);

struct Mat3x3 {
	double m[3][3];

	void setIdentity();
};



#endif

