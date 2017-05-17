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
 * @file CRecordCam.cpp
 * @author Andre Netzeband
 * @date 11.05.2017
 *
 * @brief TODO
 *
 */

#include <cstring>
#include <chrono>

#include <car.h>
#include "grcam.h"
#include "grmain.h"
#include "grscreen.h"	//cGrScreen

#include "CSharedMemroy.h"
#include "grscene.h"

#include <boost/functional/hash.hpp>

CRecordCam::CRecordCam(cGrScreen * pMyScreen,
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
    cGrCarCamMirror(pMyScreen, ID, DrawCurr, DrawBG, MyFovY, MyFovYMin, MyFovYMax, MyFNear, MyFFar, MyFogStart, MyFogEnd),
    mpScreen(pMyScreen),
    mIsFirstUpdate(true)
{
  mScreenX   = screen->getScrX();
  mScreenY   = screen->getScrY();
  mMaxWidth  = screen->getScrW();
  mMaxHeight = screen->getScrH();
  mLastTime = GfTimeClock();

  mpSharedMemory = new CSharedMemory();

  memset(&mLabelData, 0, sizeof(mLabelData));
  memset(&mGameData, 0, sizeof(mGameData));
}

CRecordCam::~CRecordCam()
{
  if (mpSharedMemory)
  {
    delete(mpSharedMemory);
  }
}

void CRecordCam::setModelView(void)
{
  sgMat4 mat;
  grMakeLookAtMat4(mat, eye, center, up);
  grContext.setCamera(mat);
}

void CRecordCam::adaptScreenSize()
{
  mScreenX   = screen->getScrX();
  mScreenY   = screen->getScrY();
  mMaxWidth  = screen->getScrW();
  mMaxHeight = screen->getScrH();

  assert(mMaxHeight - mScreenY <= RECORD_MAX_IMAGE_HEIGHT);
  assert(mMaxWidth  - mScreenX <= RECORD_MAX_IMAGE_WIDTH);
}

void CRecordCam::updateSpeed(tCarElt *pCar)
{
  speed[0] =pCar->pub.DynGCg.vel.x;
  speed[1] =pCar->pub.DynGCg.vel.y;
  speed[2] =pCar->pub.DynGCg.vel.z;

  mGameData.Speed = (pCar->_speed_x * 3.6f);

  Speed = (int)mGameData.Speed;
}

void CRecordCam::updateGameLanes()
{
  if (grTrack)
  {
    assert(grTrack->lanes >= 0);
    assert(grTrack->lanes <= 0xFF);

    mGameData.Lanes = (uint8_t) grTrack->lanes;
  }
  else
  {
    mGameData.Lanes = 0;
  }
}

void CRecordCam::updateGameTrackName()
{
  if (grTrack)
  {
    strncpy(mGameData.TrackName, grTrack->name, (size_t)MAX_TRACK_NAME_LENGTH);
    mGameData.TrackName[MAX_TRACK_NAME_LENGTH-1] = 0;
  }
  else
  {
    mGameData.TrackName[0] = 0;
  }
}

void CRecordCam::updateGameHashes()
{
  if (grTrack)
  {
    size_t Hash = 0;
    boost::hash_combine(Hash, std::string(grTrack->name));
    mGameData.UniqueTrackID = (uint64_t)Hash;
    boost::hash_combine(Hash, std::chrono::system_clock::now().time_since_epoch().count());
    mGameData.UniqueRaceID  = (uint64_t)Hash;
  }
  else
  {
    mGameData.UniqueTrackID = 0;
    mGameData.UniqueRaceID  = 0;
  }
}

void CRecordCam::updateGameData(tCarElt *pCar, tSituation *pSituation)
{
  updateSpeed(pCar);

  if (mIsFirstUpdate)
  {
    mIsFirstUpdate = false;

    updateGameLanes();
    updateGameTrackName();
    updateGameHashes();
  }
}

void CRecordCam::updateLabels(tCarElt *pCar, tSituation *pSituation)
{
}

void CRecordCam::update(tCarElt *pCar, tSituation *pSituation)
{
  updateLabels(pCar, pSituation);
  updateGameData(pCar, pSituation);
}

void CRecordCam::storeImage(int X, int Y, int Height, int Width)
{
  assert(mpSharedMemory);

  mpSharedMemory->waitOnRead();

  Data_t * const pMemory = mpSharedMemory->getAddress();
  assert(pMemory);

  int EffectiveHeight = Height - Y;
  int EffectiveWidth = Width - X;

  assert(EffectiveHeight <= mMaxHeight);
  assert(EffectiveWidth <= mMaxWidth);


  pMemory->Image.ImageHeight = EffectiveHeight;
  pMemory->Image.ImageWidth = EffectiveWidth;
  glReadPixels(X, Y, Width, Height, GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)pMemory->Image.Data);

  memcpy(&(pMemory->Labels), &mLabelData, sizeof(mLabelData));
  memcpy(&(pMemory->Game), &mGameData, sizeof(mGameData));

  mpSharedMemory->indicateWrite();
}

void CRecordCam::renderImage(tCarElt *pCar, tSituation * pSituation, uint64_t FrameNumber)
{
  double Time = GfTimeClock();
  double Difference = Time - mLastTime;

  if (Difference > (1.0/(double)RECORD_FRAME_RATE))
  {
    mLastTime = Time;
    doRender(pCar, pSituation);
  }
}


void CRecordCam::doRender(tCarElt *pCar, tSituation * pSituation)
{
  update(pCar, pSituation);
  storeImage(mScreenX, mScreenY, mMaxHeight, mMaxWidth);
}
