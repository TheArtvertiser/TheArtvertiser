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


#include <string.h>
#include "matvec.h"

void Vec3::setMul(const Mat3x4 &mat, const Vec3 &p)
{
	for (int i=0; i<3; ++i) {
		v[i] = mat.m[i][0]*p[0] +
			mat.m[i][1]*p[1] +
			mat.m[i][2]*p[2] +
			mat.m[i][3];
	}
}

void Mat3x4::setIdentity()
{
	memset(m,0,sizeof(m));
	m[0][0]=m[1][1]=m[2][2]=1;
}

void Mat3x4::set(CvMat *mat)
{
	setIdentity();

	int nr = mat->rows < 3 ? mat->rows : 3;
	int nc = mat->cols < 4 ? mat->cols : 4;

	for (int r=0; r< nr; ++r) {
		for (int c=0; c< nc; ++c) {
			m[r][c] = cvGetReal2D(mat, r, c);
		}
	}
}

void Mat3x3::setIdentity()
{
	memset(m,0,sizeof(m));
	m[0][0]=m[1][1]=m[2][2]=1;
}

void Mat3x4::setMul(const Mat3x3 &a, const Mat3x4 &b)
{
	for (int i=0; i<3; i++) {
		for (int j=0; j<4; j++) {
			m[i][j] =
			       	a.m[i][0]*b.m[0][j] +
			       	a.m[i][1]*b.m[1][j] +
			       	a.m[i][2]*b.m[2][j];
		}
	}
}

void Mat3x4::transform(const float src[4], float dst[4])
{
	float tmp[4];
	const float *s=src;
	if (src==dst) {
		memcpy(tmp, src, 4*sizeof(float));
		s=tmp;
	}

	for (int i=0; i<3; ++i) {
		dst[i]=0;
		for(int j=0;j<4; j++) 
			dst[i] += s[j]*m[i][j];
	}
}

std::ostream& operator << (std::ostream& os, const Mat3x4 &cam)
{
	for (int i=0; i<3; ++i) {
		os << "[ " << cam.m[i][0];
		for (int j=1;j<4; ++j) {
			os << ", " << cam.m[i][j];
		}
		os << " ]\n";
	}
	return os;
}

/*! Add a line [0, 0, 0, 1] to each matrix to fake 4x4 matrix multiplication.
 */
void Mat3x4::setMul(const Mat3x4 &a, const Mat3x4 &b)
{
	static const double fake[4] = {0,0,0,1};

	for (int i=0; i<3; i++) {
		for (int j=0; j<4; j++) {
			m[i][j] =
			       	a.m[i][0]*b.m[0][j] +
			       	a.m[i][1]*b.m[1][j] +
			       	a.m[i][2]*b.m[2][j] +
				a.m[i][3]*fake[j];
		}
	}
}

void Mat3x4::mul(const Mat3x4 &a)
{
	Mat3x4 nm;
	nm.setMul(*this, a);
	*this = nm;
}

void Mat3x4::setTranslate(double x, double y, double z)
{
	setIdentity();
	m[0][3] = x; 
	m[1][3] = y; 
	m[2][3] = z; 
}

void Mat3x4::setInverseByTranspose(const Mat3x4 &mat)
{
	// unrolling by maple...
	m[0][0] = mat.m[0][0];
	m[0][1] = mat.m[1][0];
	m[0][2] = mat.m[2][0];
	m[0][3] = -(mat.m[0][0]*mat.m[0][3]+mat.m[1][0]*mat.m[1]
		[3]+mat.m[2][0]*mat.m[2][3]);
	m[1][0] = mat.m[0][1];
	m[1][1] = mat.m[1][1];
	m[1][2] = mat.m[2][1];
	m[1][3] = -(mat.m[0][1]*mat.m[0][3]+mat.m[1][1]*mat.m[1]
		[3]+mat.m[2][1]*mat.m[2][3]);
	m[2][0] = mat.m[0][2];
	m[2][1] = mat.m[1][2];
	m[2][2] = mat.m[2][2];
	m[2][3] = -(mat.m[0][2]*mat.m[0][3]+mat.m[1][2]*mat.m[1]
		[3]+mat.m[2][2]*mat.m[2][3]);
}

void Mat3x4::setRotate(const Vec3 &axis, double angle)
{
	// produce a quaternion
	double theta = angle/2;
	double sinTheta = sin(theta);
	double qx = sinTheta * axis[0];
	double qy = sinTheta * axis[1];
	double qz = sinTheta * axis[2];
	double qw = cos( theta );

	double xx = qx*qx; double yy = qy*qy; double zz = qz*qz;
	double xy = qx*qy; double xz = qx*qz; double yz = qy*qz;
	double wx = qw*qx; double wy = qw*qy; double wz = qw*qz;

	m[0][0] = 1 - 2 * ( yy + zz );
	m[0][1] =     2 * ( xy - wz );
	m[0][2] =     2 * ( xz + wy );
	m[0][3] = 0;

	m[1][0] =     2 * ( xy + wz );
	m[1][1] = 1 - 2 * ( xx + zz );
	m[1][2] =     2 * ( yz - wx );
	m[1][3] = 0;

	m[2][0] =     2 * ( xz - wy );
	m[2][1] =     2 * ( yz + wx );
	m[2][2] = 1 - 2 * ( xx + yy );
	m[2][3] = 0;
}
