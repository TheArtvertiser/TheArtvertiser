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


#ifndef _LIGHTMAP_H
#define _LIGHTMAP_H

#include <vector>
#include <math/growmat.h>
#include "lightcollector.h"

//also includes opengl
#include "ipltexture.h"

/*! \defgroup photocalib Photometric Calibration
 * \ingroup garfeild
 *
 * Here's code to measure irradiance from images, to build an irradiance map
 * and to use that map to augment a scene with OpenGL.
 *
 * The most interesting class here is LightMap. An example of augmentation can
 * be found in \ref multigl.cpp.
 */
/*!
 * \ingroup photocalib
 * \brief Irradiance map computation, storage and augmentation.
 * \author Julien Pilet
 *
 * This class computes a environmental radiance map and uses OpenGL to augment
 * 3D objects accordingly.
 *
 * The process to build a light map is the following: After geometric
 * calibration, it becomes possible to turn an homography into a 3D pose (see
 * CamAugmentation). Thus, when the planar target is detected on an image, it
 * is possible to compute its normal in a reference coordinate system. It is
 * also possible to compare its color with the model one. After collecting many
 * views, a simple linear system computes gain and bias of each camera and the
 * irradiance for each normal.
 *
 * All of this is done under lambertian assumption and might not behave very
 * well in presence of specularities, specially in a multi-camera environment.
 *
 * Relighting 3D objects for augmentation can done by GPU using a vertex and a
 * pixel shader.
 */
class LightMap {
public:
	LightMap();
	~LightMap();
	LightMap(const LightMap &a);

	bool init(int nbCam, IplImage *model, float corners[4][2], int nx, int ny);
	void clear();
	void setCamNum(int n);

	bool initGL();
	void enableShader(int cam, CvMat *obj2world);
	void disableShader();

	bool load(const char *lightParamsFN="lightParams.mat", const char *normalsFN="normals.mat");
	bool save(const char *lightParamsFN="lightParams.mat", const char *normalsFN="normals.mat");

	//! calls addNormalCalib if the system is not yet solved, or addNormallightMap otherwise.
	bool addNormal(float normal[3], LightCollector &lc, int cam);

	//! Add a normal for future light map computation
	bool addNormalCalib(float normal[3], LightCollector &lc, int cam);

	//! Update the current light map with given observation
	bool addNormalLightMap(float normal[3], LightCollector &lc, int cam);

	/*! Compute the light map using all available observations, previously
	 *  added with addNormalCalib ir addNormal.
	 */
	bool computeLightParams();

	//! return true when computeLightParams() has succeeded.
	bool isReady() { return lightParams!=0; }

	//! Contains the irradiance map. Read it with readMap, update it with addNormal.
	IplTexture map;

	LightCollector reflc;

	bool saveImage(const char *filename);

	//! return the (B,G,R) gain of a camera.
	const float *getGain(int cam);
	//! return the (B,G,R) bias of a camera.
	const float *getBias(int cam);

	//! returns the irradiance for a given normal.
	CvScalar readMap(const float normal[3]);

	//! the number of normals measured that'll be used by computeLightParams.
	int nbNormals() const { return normals->rows; }

private:
	void normal2uv(const float n[3], float uv[2]);
	void uv2normal(const float uv[2], float n[3]);
	bool updateLightMap(float n[3], float *val);
	double getObsMat(int i, int j, int c);

	void buildMapFromSamples();

	// (g, b) for each cam, x for each frame
	struct Observation {
		int camCol;
		float camVal[3];
		int normalCol;
		float normalVal[3];
	};
	static double getObsElem(const std::vector<Observation>::iterator &it, int i, int c);
	void computeAtA(CvMat *AtA, int channel);

	std::vector<Observation> obs;
	CvMat *lightParams;
	CvGrowMat *normals;
	int nbCam;

	bool ARB;
	bool initialized;
	unsigned int g_vertShader, g_fragShader, g_shaderProgram;
};

#endif
