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
 * @file CLaneDriver.cpp
 * @author Andre Netzeband
 * @date 08.05.2017
 *
 * @brief TODO
 *
 */

#include <iostream>
#include <random>

#include <robottools.h>
#include <car.h>
#include <raceman.h>

#include "CLaneDriver.h"

using namespace std;

static random_device RealRandomDevice;
static default_random_engine RandomGenerator(RealRandomDevice());

static int getRandomLane(int Lanes)
{
  uniform_int_distribution<int> UniformDistribution(1, 100);
  int const Value = UniformDistribution(RandomGenerator);

  if (Lanes == 2)
  {
    int const PreferedLaneProbability = 60;

    return (Value <= PreferedLaneProbability) ? 2 : 1;
  }
  else if (Lanes == 3)
  {
    int const PreferedLaneProbability = 50;
    int const NormalLaneProbability = 100 - PreferedLaneProbability;

    if (Value <= PreferedLaneProbability)
    {
      return 2;
    }
    else
    {
      if ((Value - PreferedLaneProbability) <= NormalLaneProbability/2)
      {
        return 1;
      }
      else
      {
        return 3;
      }
    }
  }

  return Lanes;
}

CLaneDriver::CLaneDriver(char const * pName, char const * pDescription, float MaxSpeed, float MinSpeed)
{
  mName             = pName;
  mDescription      = pDescription;
  mpTrack           = NULL;
  mpCar             = NULL;
  mIsFirstDriveStep = true;
  mMaxSpeed         = MaxSpeed/3.6f;
  mMinSpeed         = MinSpeed/3.6f;
  mDriverSpeed      = 0;
  mStuckCount       = 0;
  mLaneOffset       = 0;
  mStartDelay       = 0;
}

void CLaneDriver::setupTrack(tTrack * pTrack, void * pCarHandle, void ** pCarArguments, tSituation * pRace)
{
  cout << "Setup track " << pTrack->name << " for driver " << mName << "." << endl;
  mpTrack = pTrack;
}

void CLaneDriver::setupRace(tCarElt * pCar, tSituation * pSituation)
{
  cout << "Setup a new race for driver " << mName << "." << endl;
  mpCar = pCar;
}

void CLaneDriver::choseSpeed(int StartPosition, int Drivers)
{
  float SpeedDiff  = mMaxSpeed - mMinSpeed;
  float SpeedSteps = SpeedDiff/Drivers;
  mDriverSpeed = mMaxSpeed - SpeedSteps * (StartPosition-1);

  cout << " * Driver-Speed: " << (mDriverSpeed*3.6) << "km/h from " << (mMaxSpeed*3.6) << "km/h" << endl;
}

void CLaneDriver::choseLane()
{
  switch(mpTrack->lanes)
  {
    case 2:
    {
      switch (getRandomLane(2))
      {
        case 1:
          cout << " * Drives on lane: left" << endl;
          mLaneOffset = -2.0;
          break;

        case 2:
          cout << " * Drives on lane: right" << endl;
          mLaneOffset = +2.0;
          break;
      }
      break;
    }

    case 3:
    {
      switch(getRandomLane(3))
      {
        case 1:
          cout << " * Drives on lane: left" << endl;
          mLaneOffset = -4.0;
          break;

        case 2:
          cout << " * Drives on lane: middle" << endl;
          mLaneOffset = 0.0;
          break;

        case 3:
          cout << " * Drives on lane: right" << endl;
          mLaneOffset = +4.0;
          break;
      }
      break;
    }

    default:
    case 1:
    {
      cout << " * Drives on lane: middle" << endl;
      mLaneOffset = 0.0;
      break;
    }
  }
}

void CLaneDriver::choseStartDelay(int StartPosition)
{
  mStartDelay = StartPosition*2.0f;
}

void CLaneDriver::doFirstStep(tSituation * pSituation)
{
  mIsFirstDriveStep = false;

  cout << "Initialize driver " << mName << ":" << endl;

  //int StartPosition = mpManager->getStartingPos(mDriverHandle);
  int StartPosition = mpCar->info.startRank;
  int Drivers       = pSituation->raceInfo.ncars;

  cout << " * Starting-Position: " << StartPosition << " from " << Drivers << endl;

  choseSpeed(StartPosition, Drivers);
  choseLane();
  choseStartDelay(StartPosition);
}


/// @note Original implementation taken from deep driving project.
void CLaneDriver::doSpeedControl()
{
  static double const MaxDiff = 10/3.6;

  if (mpCar->_speed_x > mDriverSpeed)
  {
    double const Diff = std::fmin(mpCar->_speed_x - mDriverSpeed, MaxDiff);

    mpCar->ctrl.brakeCmd = (tdble)((0.5 * Diff)/MaxDiff);
    mpCar->ctrl.accelCmd = 0.0;
  }
  else if (mpCar->_speed_x < mDriverSpeed)
  {
    double const Diff = std::fmin(mDriverSpeed - mpCar->_speed_x, MaxDiff);

    mpCar->ctrl.accelCmd = (tdble)((0.5 * Diff)/MaxDiff);
    mpCar->ctrl.brakeCmd = 0.0;
  }
}

/// @note Original implementation taken from deep driving project.
void CLaneDriver::doStayInLane()
{
  double Angle;
  const float SC = 1.0;

  Angle = RtTrackSideTgAngleL(&(mpCar->_trkPos)) - mpCar->_yaw;
  NORM_PI_PI(Angle); // put the angle back in the range from -PI to PI
  Angle -= SC*(mpCar->_trkPos.toMiddle+mLaneOffset)/mpCar->_trkPos.seg->width;

  // set up the values to return
  mpCar->ctrl.steer = (tdble)(Angle / mpCar->_steerLock);
}

/// @note Original implementation taken from deep driving project.
void CLaneDriver::doBackwardTurn()
{
  double angle = -RtTrackSideTgAngleL(&(mpCar->_trkPos)) + mpCar->_yaw;
  NORM_PI_PI(angle); // put the angle back in the range from -PI to PI

  mpCar->ctrl.steer = (tdble)(angle / mpCar->_steerLock);
  mpCar->ctrl.gear = -1; // reverse gear
  mpCar->ctrl.accelCmd = 0.3f; // 30% accelerator pedal
  mpCar->ctrl.brakeCmd = 0.0f; // no brakes
}

/// @note Original implementation taken from deep driving project.
int CLaneDriver::calcGear()
{
  static const float SHIFT = 0.85f;         /* [-] (% of rpmredline) */
  static const float SHIFT_MARGIN = 4.0f;   /* [m/s] */

  if (mpCar->_gear <= 0)
  {
    return 1;
  }

  float gr_up = mpCar->_gearRatio[mpCar->_gear + mpCar->_gearOffset];
  float omega = mpCar->_enginerpmRedLine/gr_up;
  float wr = mpCar->_wheelRadius(2);

  if (omega*wr*SHIFT < mpCar->_speed_x)
  {
    return mpCar->_gear + 1;
  }
  else
  {
    float gr_down = mpCar->_gearRatio[mpCar->_gear + mpCar->_gearOffset - 1];
    omega = mpCar->_enginerpmRedLine/gr_down;
    if (mpCar->_gear > 1 && omega*wr*SHIFT > mpCar->_speed_x + SHIFT_MARGIN)
    {
      return mpCar->_gear - 1;
    }
  }
  return mpCar->_gear;
}

/// @note Original implementation taken from deep driving project.
void CLaneDriver::drive(tSituation * pSituation)
{
  static const double MinTimeToSpeedIncrease = 3*60;
  memset(&mpCar->ctrl, 0, sizeof(tCarCtrl));

  if (mIsFirstDriveStep)
  {
    doFirstStep(pSituation);
  }

  if ((mDriverSpeed < mMaxSpeed) && (pSituation->currentTime > MinTimeToSpeedIncrease))
  {
    cout << "Driver " << mName << " increases speed to " << mMaxSpeed*3.6 << "km/h." << endl;
    mDriverSpeed = mMaxSpeed;
  }

  if (mpCar->priv.fuel < 0.1)
  {
    //mpCar->info.tank
    cout << "Driver " << mName << " is runnin gout of fuel" << endl;
  }

  if (pSituation->currentTime > mStartDelay)
  {
    if (isStuck())
    {
      doBackwardTurn();
    }
    else
    {
      doSpeedControl();
      doStayInLane();
      mpCar->ctrl.gear = calcGear();
    }
  }
  else
  {
    mpCar->ctrl.brakeCmd = 0.5;
  }
}

/// @note Original implementation taken from deep driving project.
bool CLaneDriver::isStuck()
{
  static const float MAX_UNSTUCK_SPEED = 5.0;
  static const float MIN_UNSTUCK_DIST = 3.0;
  static const double MAX_UNSTUCK_ANGLE = 20.0/180.0*PI;
  static const int MAX_UNSTUCK_COUNT = 250;

  double angle = RtTrackSideTgAngleL(&(mpCar->_trkPos)) - mpCar->_yaw;
  NORM_PI_PI(angle);

  if (fabs(angle) > MAX_UNSTUCK_ANGLE &&
      mpCar->_speed_x < MAX_UNSTUCK_SPEED &&
      fabs(mpCar->_trkPos.toMiddle) > MIN_UNSTUCK_DIST)
  {
    if (mStuckCount > MAX_UNSTUCK_COUNT && mpCar->_trkPos.toMiddle*angle < 0.0)
    {
      return true;
    }
    else
    {
      mStuckCount++;
      return false;
    }
  }
  else
  {
    mStuckCount = 0;
    return false;
  }
}

void CLaneDriver::endRace(tSituation * pSituation)
{
  cout << "Ends the race for driver " << mName << "." << endl;
}