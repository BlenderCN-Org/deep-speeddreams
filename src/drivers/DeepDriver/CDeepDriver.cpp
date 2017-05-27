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

#include "CDeepDriver.h"

using namespace std;

CDeepDriver::CDeepDriver(char const * pName, char const * pDescription)
{
  mName             = pName;
  mDescription      = pDescription;
  mpTrack           = NULL;
  mpCar             = NULL;
  mStuckCount       = 0;
}

void CDeepDriver::setupTrack(tTrack * pTrack, void * pCarHandle, void ** pCarArguments, tSituation * pRace)
{
  cout << "Setup track " << pTrack->name << " for driver " << mName << "." << endl;
  mpTrack = pTrack;
}

void CDeepDriver::setupRace(tCarElt * pCar, tSituation * pSituation)
{
  cout << "Setup a new race for driver " << mName << "." << endl;
  mpCar = pCar;
  memset(&(pCar->RemoteControl), 0, sizeof(pCar->RemoteControl));
}

/// @note Original implementation taken from deep driving project.
void CDeepDriver::doBackwardTurn()
{
  double angle = -RtTrackSideTgAngleL(&(mpCar->_trkPos)) + mpCar->_yaw;
  NORM_PI_PI(angle); // put the angle back in the range from -PI to PI

  mpCar->ctrl.steer = (tdble)(angle / mpCar->_steerLock);
  mpCar->ctrl.gear = -1; // reverse gear
  mpCar->ctrl.accelCmd = 0.3f; // 30% accelerator pedal
  mpCar->ctrl.brakeCmd = 0.0f; // no brakes
}

/// @note Original implementation taken from deep driving project.
int CDeepDriver::calcGear()
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
void CDeepDriver::drive(tSituation * pSituation)
{
  memset(&mpCar->ctrl, 0, sizeof(tCarCtrl));

  if (mpCar->priv.fuel < 0.1)
  {
    //mpCar->info.tank
    cout << "Driver " << mName << " is runnin gout of fuel" << endl;
  }

  if (isStuck())
  {
    doBackwardTurn();
  }
  else if (mpCar->RemoteControl.IsControlling && pSituation->raceInfo.state != RM_RACE_PRESTART)
  {
    if (mpCar->RemoteControl.pSteering)
    {
      mpCar->ctrl.steer    = (*(mpCar->RemoteControl.pSteering))/mpCar->_steerLock;
    }

    if (mpCar->RemoteControl.pAccelerating)
    {
      mpCar->ctrl.accelCmd = *(mpCar->RemoteControl.pAccelerating);
    }

    if (mpCar->RemoteControl.pBreaking)
    {
      mpCar->ctrl.brakeCmd = *(mpCar->RemoteControl.pBreaking);
    }

    mpCar->ctrl.gear = calcGear();
  }
  else
  {
    mpCar->ctrl.brakeCmd = 1.0;
  }
}

/// @note Original implementation taken from deep driving project.
bool CDeepDriver::isStuck()
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

void CDeepDriver::endRace(tSituation * pSituation)
{
  cout << "Ends the race for driver " << mName << "." << endl;
}