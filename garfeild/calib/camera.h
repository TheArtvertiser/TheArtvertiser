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

#ifndef _CAMERA_H
#define _CAMERA_H

#include "matvec.h"

/*!
 * \brief This class represents a calibrated camera.
 * \ingroup geomcalib
 *
 * The camera space is right-handed, i.e.: 
 *     x -> points right
 *     y -> points up
 *     z -> points away through screen
 *
 * The image space origin is bottom-left, and the y axis points up.
 * 
 * For square pixels s should be 0 and f==g.
 *
 * Projection equation of the 3D point P:
 *
 * \f[u = \frac{(f * Px + s * Py)}{Pz + cx}\f]
 *
 * \f[v = \frac{g * Py}{Pz + cy}\f]
 */
class PerspectiveProjection {
public:
	PerspectiveProjection();
	PerspectiveProjection(int w, int h, double f, double g, double cx, double, double s=0);
	void set(int w, int h, double f, double g, double cx , double cy, double s=0);

	void eyeToImage(const double eye[3], double uv[2]) const;
	void imageToEye(const double uv[2], double eye[3], double w=0) const;
	void imageToEye(double u, double v, double eye[3], double w=0) const;

	void setGlProjection() const;

	bool setPlanes(double near, double far);

	bool getUndistortMap(CvMat *xmap, CvMat *ymap);

//protected:
	// intrinsic parameters
	double f, g, cx, cy, s;
	double distortion;
	Mat3x3 eyeToImageMat;
	int width, height;
	double farPlane, nearPlane;

	void cmpEyeToImageMat();

	void flip();
};

/*! Perspective camera: a projection located in space.
 */
class PerspectiveCamera : public PerspectiveProjection {
public:
	PerspectiveCamera();

	void worldToImage(const double p[3], double uvw[3]) const;
	void worldToImage(const Vec3 &p, Vec3 &uvw) const;
	void worldToEye(const Vec3 &src, Vec3 &dst) const;
	void worldToEye(const double src[3], double dst[3]) const;
	void eyeToWorld(const Vec3 &uvw, Vec3 &w) const;
	void imageToWorld(double u, double v, Vec3 &w, double z=0) const;
	void setGlModelView() const;
	void setByTarget(const Vec3 pos, const Vec3 target, double roll);
	void loadTdir(const double tdir[3][4], int w, int h);
	bool loadTdir(const char *tdir, int w, int h);
	bool loadTdir(const char *tdir); // width and height are estimated. Do not trust it!
	void clearExternalParams();

	//! translate the camera relative to eye coordinates (z means "move forward").
	void translate(double dx, double dy, double dz);

	void setWorldToEyeMat(const Mat3x4 &m);
	const Mat3x4 &getWorldToEyeMat() const { return worldToEyeMat; }
	const Mat3x4 &getEyeToWorldMat() const { return eyeToWorldMat; }
	const Mat3x4 &getWorldToImageMat() const { return worldToImageMat; }

	void cmpWorldToImageMat();

	//! Save the camera in a tdir file. Returns true on success, false on failure.
	bool saveTdir(const char *file);

protected:
	// extrinsic parameters
	Mat3x4 worldToEyeMat;
	Mat3x4 eyeToWorldMat;
	Mat3x4 worldToImageMat;
};

std::ostream& operator << (std::ostream& os, const PerspectiveProjection &cam);
std::ostream& operator << (std::ostream& os, const PerspectiveCamera &cam);
	
#endif
