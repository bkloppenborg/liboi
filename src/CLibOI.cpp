/*
 * CLibOI.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include "CLibOI.h"

CLibOI::CLibOI()
{
	// TODO Auto-generated constructor stub

}

CLibOI::~CLibOI()
{
	// TODO Auto-generated destructor stub
}

/// Converts an image buffer from a OpenGL texture to an OpenCL memory object.  Also prohibits
/// the cl_mem area from being freed until explicitly released by both OpenCL and OpenGL.
cl_mem CLibOI::SetImageFromTexture(cl_context OCLContext, GLuint texture)
{
	// TODO: Permit loading of 3D textures for spectral imaging.

	int err = CL_SUCCESS;
	// TODO: note that the clCreateFromGLTexture2D was depreciated in the OpenCL 1.2 specifications.
	cl_mem image = clCreateFromGLTexture2D(OCLContext, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texture, &err);
	COpenCL::CheckOCLError("Could not create OpenCL image object from GLTexture", err);
}

cl_mem CLibOI::SetImageFromRenderbuffer(cl_context OCLContext, GLuint renderbuffer)
{
	// TODO: Permit loading of 3D textures for spectral imaging.

	int err = CL_SUCCESS;
	cl_mem image = clCreateFromGLRenderbuffer(OCLContext, CL_MEM_READ_WRITE, renderbuffer, &err);
	COpenCL::CheckOCLError("Could not create OpenCL image object from renderbuffer", err);
}
