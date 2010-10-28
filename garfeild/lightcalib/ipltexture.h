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


#ifndef _IPLTEXTURE_H
#define _IPLTEXTURE_H

#include <cv.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#ifdef WIN32
	// Comment this out if you do not have GLEW installed.
#define HAVE_GLEW
#endif
#endif

#ifdef __APPLE__ 
#undef HAVE_GLEW
#endif
#ifdef HAVE_GLEW
#include <GL/glew.h>
#ifndef HAVE_GL
#define HAVE_GL
#endif
#else
#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#define HAVE_GL
#endif

#ifdef HAVE_GL
#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif
#endif


/*!
 * \ingroup photocalib
 *
 * Represent a texture in video memory, with optional caching.
 *
 * Honours many IplImage fields like nChannels, depth, align, and origin.
 */
class IplTexture {

public:
	IplTexture(IplImage *image=0, bool cache=true, bool smooth=true) 
		: im(image), downsampled(0), allowCache(cache), reload(true),  
		smooth(smooth), textureGenerated(false), refcnt(1) {}
	
	virtual ~IplTexture();
		
	//! Only call genTexture from a valid OpenGL context !
	void genTexture();
	void loadTexture();
	void disableTexture();
	void update() { reload=true; }
	void setImage(IplImage *image);
	IplImage *getImage() { return im; }
	IplImage *getIm() { return im; }
	const IplImage *getIm() const { return im; }
	void freeImage() { if (this && im) { cvReleaseImage(&im); } }

	//! Get the U texel coordinates from pixel coordinate x.
	double u(double x) { return x*uScale; }

	//! Get the V texel coordinates from pixel coordinate y (axis pointing down).
	double v(double y) { return y*vScale + vOrigin; }

	//! force texture regeneration.
	void regen(); 

	//! Add a reference to the reference counter.
	void addRef() { refcnt++; }

	/*! Removes a reference to the reference counter, and delete the
	 *  texture if it reaches 0.
	 */
	void unref();
	
	void clearWithoutDelete() { im = downsampled = 0; }

private:
	IplImage *im;
	IplImage *downsampled;

	bool allowCache;
	bool reload;
	bool smooth;
	bool textureGenerated;
	int refcnt;
	unsigned int texture;
	double uScale, vScale, vOrigin;
	int texWidth, texHeight;
};

#endif
