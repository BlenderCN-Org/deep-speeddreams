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
#include "CRecordCam.h"

#include <boost/functional/hash.hpp>
#include <raceman.h>
#include <tgf.h>

#include "robottools.h"

/// @brief The width of a line in m.
static tdble const sLaneWidth = 4.0;
static tdble const sMinLL = -7.5;
static tdble const sMaxLL = -4.5;
static tdble const sInvalidLL = -9;
static tdble const sMinML = -3.5;
static tdble const sMaxML = -0.5;
static tdble const sInvalidML = -5;
static tdble const sMinMR =  0.5;
static tdble const sMaxMR =  3.5;
static tdble const sInvalidMR = 5;
static tdble const sMinRR =  4.5;
static tdble const sMaxRR =  7.5;
static tdble const sInvalidRR = 9;
static tdble const sMinL  = -5;
static tdble const sMaxL  = -3;
static tdble const sInvalidL = -7;
static tdble const sMinM  = -1.2;
static tdble const sMaxM  = 1.2;
static tdble const sInvalidM = -5;
static tdble const sMinR  = 3;
static tdble const sMaxR  = 5;
static tdble const sInvalidR = 7;
static tdble const sMaxObstacleDist = 60.0;
static tdble const sInvalidDist = 90;

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

  mStreet.pLane = 0;
}

static void delStreetDescription(StreetDescription_t * pStreet)
{
  if (pStreet->pLane)
  {
    delete[](pStreet->pLane);
  }
  pStreet->pLane = 0;

  pStreet->Lanes = 0;
}

CRecordCam::~CRecordCam()
{
  if (mpSharedMemory)
  {
    delete(mpSharedMemory);
  }

  delStreetDescription(&mStreet);
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

  mGameData.Speed = pCar->_speed_x;

  Speed = (int)(pCar->_speed_x * 3.6f);
}

static void calcStreetDescription(StreetDescription_t * pStreet, int Lanes)
{
  delStreetDescription(pStreet);

  pStreet->Lanes    = Lanes;
  pStreet->pLane    = new LaneDescription_t[Lanes];

  tdble const StreetWidth = sLaneWidth * Lanes;

  for (int i = 0; i < Lanes; i++)
  {
    tdble const LaneStart = sLaneWidth * i;
    tdble const LaneEnd   = sLaneWidth * (i+1);

    pStreet->pLane[i].LaneStartFromMiddle = LaneStart - StreetWidth/2.0f;
    pStreet->pLane[i].LaneEndFromMiddle   = LaneEnd   - StreetWidth/2.0f;
    pStreet->pLane[i].ObstacleDistance    = sInvalidDist;
  }
}


void CRecordCam::updateGameLanes()
{
  if (grTrack)
  {
    assert(grTrack->lanes >= 0);
    assert(grTrack->lanes <= 0xFF);

    calcStreetDescription(&mStreet, grTrack->lanes);
    mGameData.Lanes = (uint8_t)mStreet.Lanes;
  }
  else
  {
    delStreetDescription(&mStreet);
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


static tdble calcDistanceFromSegmentStart(tCarElt const * pCar)
{
  if (pCar->pub.trkPos.seg->type == TR_STR)
  {
    return pCar->pub.trkPos.toStart;
  }
  else
  {
    // If the segment is a turn, "toStart" is given in ARC instead of m.
    return pCar->pub.trkPos.toStart * pCar->pub.trkPos.seg->radius;
  }
}


static int getLaneOfCar(tCarElt const * const pCar, StreetDescription_t const * const pStreet)
{
  tdble const DistanceToStreetMiddle = -pCar->pub.trkPos.toMiddle;

  for(int i = 0; i < pStreet->Lanes; i++)
  {
    if (DistanceToStreetMiddle < pStreet->pLane[i].LaneStartFromMiddle)
    {
      return i-1;
    }
    else if (DistanceToStreetMiddle < pStreet->pLane[i].LaneEndFromMiddle)
    {
      return i;
    }
  }

  return pStreet->Lanes+1;
}

static int getLeftLaneOfCar(tCarElt const * const pCar, StreetDescription_t const * const pStreet)
{
  tdble const DistanceToStreetMiddle = -pCar->pub.trkPos.toMiddle - pCar->info.dimension.y/2;

  for(int i = 0; i < pStreet->Lanes; i++)
  {
    if (DistanceToStreetMiddle < pStreet->pLane[i].LaneStartFromMiddle)
    {
      return i-1;
    }
    else if (DistanceToStreetMiddle < pStreet->pLane[i].LaneEndFromMiddle)
    {
      return i;
    }
  }

  return pStreet->Lanes+1;
}

static int getRightLaneOfCar(tCarElt const * const pCar, StreetDescription_t const * const pStreet)
{
  tdble const DistanceToStreetMiddle = -pCar->pub.trkPos.toMiddle + pCar->info.dimension.y/2;

  for(int i = 0; i < pStreet->Lanes; i++)
  {
    if (DistanceToStreetMiddle < pStreet->pLane[i].LaneStartFromMiddle)
    {
      return i-1;
    }
    else if (DistanceToStreetMiddle < pStreet->pLane[i].LaneEndFromMiddle)
    {
      return i;
    }
  }

  return pStreet->Lanes+1;
}

static tdble getDistanceFromStart(tCarElt const * pCar)
{
  tdble const DistanceOfSegmentFromStart = pCar->pub.trkPos.seg->lgfromstart;
  tdble const DistanceFromSegmentStart = calcDistanceFromSegmentStart(pCar);
  return DistanceOfSegmentFromStart + DistanceFromSegmentStart;
}

static tdble getDistanceBetweenCars(tCarElt const * pCar1, tCarElt const * pCar2)
{
  assert(grTrack);

  tdble const DistanceFromStartCar1 = getDistanceFromStart(pCar1);
  tdble const DistanceFromStartCar2 = getDistanceFromStart(pCar2);
  tdble Distance = DistanceFromStartCar2 - DistanceFromStartCar1;

  tdble const TrackLength = grTrack->length;

  if (Distance > TrackLength/2.0)
  {
    Distance -= TrackLength;
  }
  else if (Distance < -TrackLength/2.0)
  {
    Distance += TrackLength;
  }

  return Distance;
}

static void calcObstacleDistances(StreetDescription_t * pStreet, tCarElt *pCurrentCar, tSituation *pSituation)
{
  for (int i = 0; i < pStreet->Lanes; i++)
  {
    pStreet->pLane[i].ObstacleDistance = sInvalidDist;
  }

  int const NumberOfCars = pSituation->raceInfo.ncars;
  for (int i = 0; i < NumberOfCars; i++)
  {
    tCarElt const * const pCar = pSituation->cars[i];

    if (pCar != pCurrentCar)
    {
      tdble Distance = getDistanceBetweenCars(pCurrentCar, pCar);

      if (Distance >= 0.0 && Distance <= sMaxObstacleDist)
      {
        int const LeftCarLane = getLeftLaneOfCar(pCar, pStreet);
        int const RightCarLane = getRightLaneOfCar(pCar, pStreet);

        assert(LeftCarLane <= RightCarLane);

        if (LeftCarLane >= 0 && LeftCarLane < pStreet->Lanes)
        {
          if (pStreet->pLane[LeftCarLane].ObstacleDistance > Distance)
          {
            pStreet->pLane[LeftCarLane].ObstacleDistance = Distance;
          }
        }

        if (RightCarLane >= 0 && RightCarLane < pStreet->Lanes && LeftCarLane != RightCarLane)
        {
          if (pStreet->pLane[RightCarLane].ObstacleDistance > Distance)
          {
            pStreet->pLane[RightCarLane].ObstacleDistance = Distance;
          }
        }
      }
    }
  }
}

static bool isSegmentStraight(tTrackSeg * pSegment)
{
  if (pSegment->type == TR_STR)
  {
    return true;
  }
  else if (pSegment->radius > 100)
  {
    return true;
  }

  return false;
}

void CRecordCam::updateLabelsFast(tCarElt *pCar)
{
  mLabelData.Fast = 0.0;

  if (isSegmentStraight(pCar->pub.trkPos.seg))
  {
    tdble StraightLength = 0;

    if (pCar->pub.trkPos.type == TR_STR)
    {
      StraightLength -= pCar->pub.trkPos.toStart;
    }
    else
    {
      StraightLength -= -pCar->pub.trkPos.toStart * pCar->pub.trkPos.seg->radius;
    }

    tTrackSeg * pSegment = pCar->pub.trkPos.seg;

    while (isSegmentStraight(pSegment))
    {
      StraightLength += pSegment->length;
      pSegment = pSegment->next;

      if (StraightLength >= 60.0)
      {
        mLabelData.Fast = 1.0;
        return;
      }
    }
  }
}


void CRecordCam::updateLabelsAngle(tCarElt *pCar)
{
  double Angle = RtTrackSideTgAngleL(&(pCar->pub.trkPos)) - pCar->pub.DynGC.pos.az;
  NORM_PI_PI(Angle);
  mLabelData.Angle = (float)Angle;
}


void CRecordCam::updateInLaneSystem(tCarElt *pCar)
{
  int const CurrentLane = getLaneOfCar(pCar, &mStreet);
  tdble const DistanceToMiddle = -pCar->pub.trkPos.toMiddle;

  mLabelData.LL     = sInvalidLL;
  mLabelData.ML     = sInvalidML;
  mLabelData.MR     = sInvalidMR;
  mLabelData.RR     = sInvalidRR;
  mLabelData.DistLL = sInvalidDist;
  mLabelData.DistMM = sInvalidDist;
  mLabelData.DistRR = sInvalidDist;

  // update left lane information
  if (CurrentLane >= 1 && CurrentLane < mStreet.Lanes)
  {
    LaneDescription_t const * const pLane = &(mStreet.pLane[CurrentLane-1]);

    tdble const LL = pLane->LaneStartFromMiddle - DistanceToMiddle;

    if (LL >= sMinLL && LL <= sMaxLL)
    {
      mLabelData.LL     = LL;
      mLabelData.DistLL = pLane->ObstacleDistance;
    }
  }

  // update middle lane information
  if (CurrentLane >= 0 && CurrentLane < mStreet.Lanes)
  {
    LaneDescription_t const * const pLane = &(mStreet.pLane[CurrentLane]);

    tdble const ML = pLane->LaneStartFromMiddle - DistanceToMiddle;
    tdble const MR = pLane->LaneEndFromMiddle - DistanceToMiddle;

    if ((ML >= sMinML && ML <= sMaxML) && (MR >= sMinMR && MR <= sMaxMR))
    {
      mLabelData.ML     = ML;
      mLabelData.MR     = MR;
      mLabelData.DistMM = pLane->ObstacleDistance;
    }
  }

  // update right lane information
  if (CurrentLane >= 0 && CurrentLane < mStreet.Lanes-1)
  {
    LaneDescription_t const * const pLane = &(mStreet.pLane[CurrentLane+1]);

    tdble const RR = pLane->LaneEndFromMiddle - DistanceToMiddle;

    if (RR >= sMinRR && RR <= sMaxRR)
    {
      mLabelData.RR     = RR;
      mLabelData.DistRR = pLane->ObstacleDistance;
    }
  }
}

void CRecordCam::updateOnLaneMarkingSystem(tCarElt *pCar)
{
  mLabelData.L = sInvalidL;
  mLabelData.M = sInvalidM;
  mLabelData.R = sInvalidR;
  mLabelData.DistL = sInvalidDist;
  mLabelData.DistR = sInvalidDist;

  int const CurrentLane = getLaneOfCar(pCar, &mStreet);
  tdble const DistanceToMiddle = -pCar->pub.trkPos.toMiddle;

  LaneDescription_t const * pLaneL = NULL;
  LaneDescription_t const * pLaneR = NULL;

  if (CurrentLane < 0)
  {
    pLaneL = NULL;
    pLaneR = &(mStreet.pLane[0]);
  }
  else if (CurrentLane >= mStreet.Lanes)
  {
    pLaneL = &(mStreet.pLane[mStreet.Lanes-1]);
    pLaneR = NULL;
  }
  else
  {
    LaneDescription_t const * const pLaneM = &(mStreet.pLane[CurrentLane]);
    tdble const ML = pLaneM->LaneStartFromMiddle - DistanceToMiddle;
    tdble const MR = pLaneM->LaneEndFromMiddle   - DistanceToMiddle;

    if (-ML < MR)
    {
      pLaneR = pLaneM;

      if (CurrentLane > 0)
      {
        pLaneL = &(mStreet.pLane[CurrentLane-1]);
      }
      else
      {
        pLaneL = NULL;
      }
    }
    else
    {
      pLaneL = pLaneM;

      if (CurrentLane < mStreet.Lanes-1)
      {
        pLaneR = &(mStreet.pLane[CurrentLane+1]);
      }
      else
      {
        pLaneR = NULL;
      }
    }
  }

  if (pLaneL)
  {
    tdble const M = pLaneL->LaneEndFromMiddle - DistanceToMiddle;

    if (M >= sMinM && M <= sMaxM)
    {
      mLabelData.L     = pLaneL->LaneStartFromMiddle - DistanceToMiddle;
      mLabelData.M     = M;
      mLabelData.DistL = pLaneL->ObstacleDistance;
    }
  }

  if (pLaneR)
  {
    tdble const M = pLaneR->LaneStartFromMiddle - DistanceToMiddle;

    if (M >= sMinM && M <= sMaxM)
    {
      mLabelData.R     = pLaneR->LaneEndFromMiddle - DistanceToMiddle;
      mLabelData.M     = M;
      mLabelData.DistR = pLaneR->ObstacleDistance;
    }
  }
}

void CRecordCam::updateLabels(tCarElt *pCar, tSituation *pSituation)
{
  calcObstacleDistances(&mStreet, pCar, pSituation);

  updateLabelsFast(pCar);
  updateLabelsAngle(pCar);
  updateInLaneSystem(pCar);
  updateOnLaneMarkingSystem(pCar);
}

void CRecordCam::update(tCarElt *pCar, tSituation *pSituation)
{
  updateGameData(pCar, pSituation);
  updateLabels(pCar, pSituation);
}

void CRecordCam::storeImage(int X, int Y, int Height, int Width, tCarElt *pCar)
{
  assert(mpSharedMemory);

  Data_t * const pMemory = mpSharedMemory->getAddress();
  assert(pMemory);

  pCar->RemoteControl.IsControlling = pMemory->Control.IsControlling;
  if (pMemory->Control.IsControlling)
  {
    pCar->RemoteControl.pSteering     = &(pMemory->Control.Steering);
    pCar->RemoteControl.pAccelerating = &(pMemory->Control.Accelerating);
    pCar->RemoteControl.pBreaking     = &(pMemory->Control.Breaking);
  }

  int EffectiveHeight = Height - Y;
  int EffectiveWidth = Width - X;

  assert(EffectiveHeight <= mMaxHeight);
  assert(EffectiveWidth <= mMaxWidth);

  pMemory->Image.ImageHeight = EffectiveHeight;
  pMemory->Image.ImageWidth = EffectiveWidth;
  glReadPixels(X, Y, Width, Height, GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)pMemory->Image.Data);

  memcpy(&(pMemory->Labels), &mLabelData, sizeof(mLabelData));
  memcpy(&(pMemory->Game), &mGameData, sizeof(mGameData));

  pCar->RemoteControl.IsControlling = pMemory->Control.IsControlling;
  if (pCar->RemoteControl.IsControlling == 0)
  {
    pMemory->Control.Steering     = pCar->ctrl.steer;
    pMemory->Control.Accelerating = pCar->ctrl.accelCmd;
    pMemory->Control.Breaking     = pCar->ctrl.brakeCmd;
  }

  mpSharedMemory->indicateWrite();
  mpSharedMemory->waitOnRead();

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
  storeImage(mScreenX, mScreenY, mMaxHeight, mMaxWidth, pCar);
}
