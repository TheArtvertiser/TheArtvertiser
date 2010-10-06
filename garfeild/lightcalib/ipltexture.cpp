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


#include <iostream>
#include "ipltexture.h"

IplTexture::~IplTexture() 
{
	if (downsampled) cvReleaseImage(&downsampled);
}

void IplTexture::genTexture()
{
#ifdef HAVE_GL
	if (im==0) return;

	if (!textureGenerated) {
		glGenTextures(1,(GLuint*) &texture);
		textureGenerated = true;
	}
	glBindTexture(GL_TEXTURE_2D, texture);

	for (texWidth=1; texWidth < im->width; texWidth <<=1);
	for (texHeight=1; texHeight < im->height; texHeight <<=1);

	if (texWidth > 1024) texWidth = 1024;
	if (texHeight > 1024) texHeight = 1024;
	//std::cout << "IplTexture *: "<< this << ": generating a " << texWidth << "x" << texHeight << " texture for a "
	//	<< im->width << "x" << im->height << " image.\n";

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (smooth ? GL_LINEAR : GL_NEAREST));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (smooth ? GL_LINEAR : GL_NEAREST));

	int sz = texWidth*texHeight*4;
	char *buffer = (char *) malloc(sz);
	memset(buffer, 0, sz);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth,texHeight, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, buffer);
	free(buffer);
#endif
}

void IplTexture::loadTexture()
{
#ifdef HAVE_GL
	if (im==0) return;
	if (!textureGenerated) genTexture();

	IplImage *im = (downsampled ? downsampled : this->im);

	if ((im->width > texWidth) || (im->height > texHeight)) {
		if (downsampled) cvReleaseImage(&downsampled);
		downsampled = cvCreateImage(cvSize(texWidth, texHeight), this->im->depth, this->im->nChannels);
		im = downsampled;
	}

	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	if (allowCache && !reload) return;
	reload = false;

	if (downsampled)
		cvResize(this->im, downsampled, CV_INTER_AREA);

	GLenum format;
	GLenum type;
	switch (im->depth) {
		case IPL_DEPTH_8U: type = GL_UNSIGNED_BYTE; break;
		case IPL_DEPTH_8S: type = GL_BYTE; break; 
		case IPL_DEPTH_16S: type = GL_SHORT; break;
		case IPL_DEPTH_32F: type = GL_FLOAT; break;
		default:
				    std::cerr << "IplTexture::loadTexture(): unsupported pixel type.\n";
				    return;
	}
	switch (im->nChannels) {
		case 1: format = GL_LUMINANCE; break;
		case 3: format = (im->channelSeq[0] == 'B') ? GL_BGR_EXT : GL_RGB; break;
		case 4: format = GL_RGBA; break;
		default:
			std::cerr << "IplTexture::loadTexture(): unsupported number of channels.\n";
			return;
	}

	// pixel storage mode
	glPixelStorei(GL_UNPACK_ALIGNMENT, im->align);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, im->width, im->height,
			format, type, im->imageData);

	uScale = (double(im->width)/double(this->im->width))/double(texWidth);
	vScale = (double(im->height)/double(this->im->height))/double(texHeight);
	vOrigin = 0;

	if (im->origin) {
		vScale = -vScale;
		vOrigin = double(im->height)/double(texHeight);
	}
#endif
}

void IplTexture::disableTexture()
{
#ifdef HAVE_GL
	glDisable(GL_TEXTURE_2D);
#endif
}

void IplTexture::unref()
{
	refcnt--;
	if (refcnt <= 0)
		delete this;
}

void IplTexture::setImage(IplImage *image)
{
	im = image;
	update();
}

void IplTexture::regen()
{
       	update(); 
	textureGenerated = false; 
}

