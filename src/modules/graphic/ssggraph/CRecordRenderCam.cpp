/**
 *  Copyright (C) 2017 Andre Netzeband
 * 
 *  This file is part of "DeepDriving for Speed-Dreams".
 *
 *  "DeepDriving for Speed-Dreams" is free software: you can redistribute it 
 *  and/or modify it under the terms of the GNU General Public License as 
 *  published by the Free Software Foundation, either version 3 of the License, 
 *  or (at your option) any later version.
 *
 *  "DeepDriving for Speed-Dreams" is distributed in the hope that it 
 *  will be useful, but WITHOUT ANY WARRANTY; without even the implied 
 *  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with "DeepDriving for Speed-Dreams".  
 *  If not, see <http://www.gnu.org/licenses/>.  
 */

/**
 * @file CRecordRenderCam.cpp
 * @author Andre Netzeband
 * @date 12.05.2017
 *
 * @brief TODO
 *
 */

#include "CRecordRenderCam.h"

#include "grcam.h"
#include "grmain.h"
#include "grscreen.h"	//cGrScreen

#include <GL/glext.h>

/// the following definitions are taken from: https://searchcode.com/codesearch/view/8069990/
/// this code is published by Gregor Kalinik under GPL license
typedef void (APIENTRY *glGenFramebuffers_t) (GLsizei n, const GLuint *);
typedef void (APIENTRY *glDeleteFramebuffers_t) (GLsizei, const GLuint *);
typedef void (APIENTRY *glBindFramebuffer_t) (GLenum target, GLuint framebuffer);

typedef void (APIENTRY *glGenRenderbuffers_t) (GLsizei, GLuint *);
typedef void (APIENTRY *glDeleteRenderbuffers_t) (GLsizei, const GLuint *);
typedef void (APIENTRY *glBindRenderbuffer_t) (GLenum, GLuint);
typedef void (APIENTRY *glRenderbufferStorage_t) (GLenum, GLenum, GLsizei, GLsizei);

typedef void (APIENTRY *glFramebufferRenderbuffer_t) (GLenum, GLenum, GLenum, GLuint);
typedef void (APIENTRY *glDrawBuffers_t) (GLsizei, const GLenum *);
typedef GLenum (APIENTRY *glCheckFramebufferStatus_t) (GLenum);

static glGenFramebuffers_t         glGenFramebuffers         = (glGenFramebuffers_t)wglGetProcAddress("glGenFramebuffers");
static glDeleteFramebuffers_t      glDeleteFramebuffers      = (glDeleteFramebuffers_t)wglGetProcAddress("glDeleteFramebuffers");
static glBindFramebuffer_t         glBindFramebuffer         = (glBindFramebuffer_t)wglGetProcAddress("glBindFramebuffer");

static glGenRenderbuffers_t        glGenRenderbuffers        = (glGenRenderbuffers_t)wglGetProcAddress("glGenRenderbuffers");
static glDeleteRenderbuffers_t     glDeleteRenderbuffers     = (glDeleteRenderbuffers_t)wglGetProcAddress("glDeleteRenderbuffers");
static glBindRenderbuffer_t        glBindRenderbuffer        = (glBindRenderbuffer_t)wglGetProcAddress("glBindRenderbuffer");
static glRenderbufferStorage_t     glRenderbufferStorage     = (glRenderbufferStorage_t)wglGetProcAddress("glRenderbufferStorage");

static glFramebufferRenderbuffer_t glFramebufferRenderbuffer = (glFramebufferRenderbuffer_t)wglGetProcAddress("glFramebufferRenderbuffer");
static glDrawBuffers_t             glDrawBuffers             = (glDrawBuffers_t)wglGetProcAddress("glDrawBuffers");
static glCheckFramebufferStatus_t  glCheckFramebufferStatus  = (glCheckFramebufferStatus_t)wglGetProcAddress("glCheckFramebufferStatus");

static bool setupOpenGLFunctions()
{
  glGenFramebuffers         = (glGenFramebuffers_t)wglGetProcAddress("glGenFramebuffers");
  printf("Found function glGenFramebuffers: %d\n", glGenFramebuffers != NULL);
  glDeleteFramebuffers      = (glDeleteFramebuffers_t)wglGetProcAddress("glDeleteFramebuffers");
  printf("Found function glDeleteFramebuffers: %d\n", glDeleteFramebuffers != NULL);
  glBindFramebuffer         = (glBindFramebuffer_t)wglGetProcAddress("glBindFramebuffer");
  printf("Found function glBindFramebuffer: %d\n", glBindFramebuffer != NULL);

  glGenRenderbuffers        = (glGenRenderbuffers_t)wglGetProcAddress("glGenRenderbuffers");
  printf("Found function glGenRenderbuffers: %d\n", glGenRenderbuffers != NULL);
  glDeleteRenderbuffers     = (glDeleteRenderbuffers_t)wglGetProcAddress("glDeleteRenderbuffers");
  printf("Found function glDeleteRenderbuffers: %d\n", glDeleteRenderbuffers != NULL);
  glBindRenderbuffer        = (glBindRenderbuffer_t)wglGetProcAddress("glBindRenderbuffer");
  printf("Found function glBindRenderbuffer: %d\n", glBindRenderbuffer != NULL);
  glRenderbufferStorage     = (glRenderbufferStorage_t)wglGetProcAddress("glRenderbufferStorage");
  printf("Found function glRenderbufferStorage: %d\n", glRenderbufferStorage != NULL);

  glFramebufferRenderbuffer = (glFramebufferRenderbuffer_t)wglGetProcAddress("glFramebufferRenderbuffer");
  printf("Found function glFramebufferRenderbuffer: %d\n", glFramebufferRenderbuffer != NULL);
  glDrawBuffers             = (glDrawBuffers_t)wglGetProcAddress("glDrawBuffers");
  printf("Found function glDrawBuffers: %d\n", glDrawBuffers != NULL);
  glCheckFramebufferStatus  = (glCheckFramebufferStatus_t)wglGetProcAddress("glCheckFramebufferStatus");
  printf("Found function glCheckFramebufferStatus: %d\n", glCheckFramebufferStatus != NULL);

  return glGenFramebuffers &&
         glDeleteFramebuffers &&
         glBindFramebuffer &&
         glGenRenderbuffers &&
         glDeleteRenderbuffers &&
         glBindRenderbuffer &&
         glRenderbufferStorage &&
         glFramebufferRenderbuffer &&
         glDrawBuffers &&
         glCheckFramebufferStatus;
}

CRecordRenderCam::CRecordRenderCam(cGrScreen * pMyScreen,
                             int ID,
                             int DrawCurr,
                             int DrawBG,
                             float MyFovY,
                             float MyFovYMin,
                             float MyFovYMax,
                             float MyFNear,
                             float MyFFar,
                             float MyFogStart,
                             float MyFogEnd):
    CRecordCam(pMyScreen, ID, DrawCurr, DrawBG, MyFovY, MyFovYMin, MyFovYMax, MyFNear, MyFFar, MyFogStart, MyFogEnd)
{
  bool SetupSucessful = setupOpenGLFunctions();

  assert(SetupSucessful);

  // create frame buffer
  mFramebufferName = 0;
  glGenFramebuffers(1, &mFramebufferName);
  glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferName);

  // color renderbuffer
  mColorBuffer = 0;
  glGenRenderbuffers(1, &mColorBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, mColorBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, RECORD_IMAGE_WIDTH, RECORD_IMAGE_HEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mColorBuffer);

  // The depth buffer
  mDepthBuffer = 0;
  glGenRenderbuffers(1, &mDepthBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, RECORD_IMAGE_WIDTH, RECORD_IMAGE_HEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);

  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

CRecordRenderCam::~CRecordRenderCam()
{
  glDeleteRenderbuffers(1, &mDepthBuffer);
  glDeleteRenderbuffers(1, &mColorBuffer);
  glDeleteFramebuffers(1, &mFramebufferName);
}

void CRecordRenderCam::setModelView(void)
{
  sgMat4 mat;

  grMakeLookAtMat4(mat, eye, center, up);
  grContext.setCamera(mat);
}


void CRecordRenderCam::update(tCarElt *car, tSituation *s) {
  sgVec3 P, p;
  float offset = 0;
  float const LookSideAngle = (((-90)*(float)PI)/180.0f);
  float const LookDownAngle = (((-50)*(float)PI)/180.0f);

  sgVec3 CameraPos;

  CameraPos[0] = car->_drvPos_x;
  CameraPos[1] = (car->_dimension_y / 2);
  CameraPos[2] = car->_statGC_z + 1.0f;

  p[0] = CameraPos[0];
  p[1] = CameraPos[1];
  p[2] = CameraPos[2];
  sgXformPnt3(p, car->_posMat);

  eye[0] = p[0];
  eye[1] = p[1];
  eye[2] = p[2];

  // Compute offset angle and bezel compensation)
  if (viewOffset) {
    //offset += getSpanAngle();
  }

  P[0] = CameraPos[0] + 30.0f * cos(offset + LookSideAngle) * cos(LookDownAngle);
  P[1] = CameraPos[1] - 30.0f * sin(offset + LookSideAngle) * cos(LookDownAngle);
  P[2] = CameraPos[2] + 30.0f * sin(LookDownAngle);

  sgXformPnt3(P, car->_posMat);

  center[0] = P[0];
  center[1] = P[1];
  center[2] = P[2];

  up[0] = car->_posMat[2][0];
  up[1] = car->_posMat[2][1];
  up[2] = car->_posMat[2][2];

  CRecordCam::update(car, s);
}


void CRecordRenderCam::beforeDraw (void)
{
  glPushAttrib(GL_VIEWPORT_BIT);

  glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferName);
  glViewport(0, 0, RECORD_IMAGE_WIDTH, RECORD_IMAGE_HEIGHT);

  // make mirror in front of everything by forcing overdrawing of everything
  glClear(GL_DEPTH_BUFFER_BIT);
}


void CRecordRenderCam::afterDraw (void)
{
  storeImage(0, 0, RECORD_IMAGE_HEIGHT, RECORD_IMAGE_WIDTH);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glPopAttrib();
}

void CRecordRenderCam::doRender(tCarElt *pCar, tSituation * pSituation)
{
  mpScreen->camDraw(pSituation);
}