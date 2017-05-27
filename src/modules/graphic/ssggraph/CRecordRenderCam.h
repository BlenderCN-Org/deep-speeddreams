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
 * @file CRecordRenderCam.h
 * @author Andre Netzeband
 * @date 12.05.2017
 *
 * @brief A camera that performs an own render pass.
 *
 */

#ifndef SPEED_DREAMS_2_CRECORDRENDERCAM_H
#define SPEED_DREAMS_2_CRECORDRENDERCAM_H

#include "CRecordCam.h"

class CRecordRenderCam : public CRecordCam
{
  public:
    CRecordRenderCam(cGrScreen * pMyScreen,
                  int ID,
                  int DrawCurr,
                  int DrawBG,
                  float MyFovY,
                  float MyFovYMin,
                  float MyFovYMax,
                  float MyFNear,
                  float MyFFar = 1500.0,
                  float MyFogStart = 1400.0,
                  float MyFogEnd = 1500.0);

    virtual ~CRecordRenderCam();

    virtual void setModelView();
    virtual void update(tCarElt *car, tSituation *s);
    virtual void afterDraw (void);
    virtual void beforeDraw (void);
    virtual void doRender(tCarElt *pCar, tSituation * pSituation);

  protected:
    GLuint mFramebufferName;
    GLuint mColorBuffer;
    GLuint mDepthBuffer;
    tCarElt *mpCurrentCar;
};


#endif //SPEED_DREAMS_2_CRECORDRENDERCAM_H
