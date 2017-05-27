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
 * @file CRecordCam.h
 * @author Andre Netzeband
 * @date 11.05.2017
 *
 * @brief Is a camera object to record the current scene.
 *
 */

#include "grcam.h"
#include "CSharedMemroy.h"

#ifndef SPEED_DREAMS_2_CRECORDCAM_H
#define SPEED_DREAMS_2_CRECORDCAM_H

#define RECORD_IMAGE_WIDTH  640
#define RECORD_IMAGE_HEIGHT 480
#define RECORD_FRAME_RATE 10

class CSharedMemory;


typedef struct
{
  tdble LaneStartFromMiddle;
  tdble LaneEndFromMiddle;
  tdble ObstacleDistance;
} LaneDescription_t;

typedef struct
{
  int Lanes;
  LaneDescription_t     * pLane;
} StreetDescription_t;


class CRecordCam : public cGrCarCamMirror
{
  public:
    CRecordCam(cGrScreen * pMyScreen,
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

    virtual ~CRecordCam();

    virtual void adaptScreenSize();
    virtual void update(tCarElt *pCar, tSituation *pSituation);
    virtual void setModelView();
    virtual void renderImage(tCarElt *pCar, tSituation * pSituation, uint64_t FrameNumber);

  protected:
    virtual void storeImage(int X, int Y, int Height, int Width, tCarElt *pCar);
    virtual void doRender(tCarElt *pCar, tSituation * pSituation);

    void updateGameData(tCarElt *pCar, tSituation *pSituation);
    void updateSpeed(tCarElt *pCar);
    void updateGameLanes();
    void updateGameTrackName();
    void updateGameHashes();
    void updateLabels(tCarElt *pCar, tSituation *pSituation);
    void updateLabelsFast(tCarElt *pCar);
    void updateLabelsAngle(tCarElt *pCar);
    void updateInLaneSystem(tCarElt *pCar);
    void updateOnLaneMarkingSystem(tCarElt *pCar);

    cGrScreen * mpScreen;
    int mScreenX;
    int mScreenY;
    int mMaxHeight;
    int mMaxWidth;
    CSharedMemory * mpSharedMemory;
    double mLastTime;
    Game_t    mGameData;
    Labels_t  mLabelData;
    bool mIsFirstUpdate;
    StreetDescription_t mStreet;
};

#endif //SPEED_DREAMS_2_CRECORDCAM_H
