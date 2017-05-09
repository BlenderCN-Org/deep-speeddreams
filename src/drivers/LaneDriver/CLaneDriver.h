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
 * @file CLaneDriver.h
 * @author Andre Netzeband
 * @date 08.05.2017
 *
 * @brief A robot that simply drives on a single lane.
 *        The lane and speed is chosen automatically
 *        dependent on the starting position of the
 *        robot car.
 *
 * @note Several parts of this code is taken from the Deep Driving project from Chenyi Chen.
 *       When using this file, please also consider the original license.
 *       URL: http://deepdriving.cs.princeton.edu/
 *
 */

#include <string>

#include <track.h>
#include <car.h>
#include <robot.h>

#ifndef SPEED_DREAMS_2_CLANEDRIVER_H
#define SPEED_DREAMS_2_CLANEDRIVER_H


class CLaneDriver
{
  public:
    /// @brief Constructor.
    /// @param pName        Is the name of the driver.
    /// @param pDescription Is the description of the driver.
    /// @param pManager     Is a driver manager object.
    /// @param MaxSpeed     Is the maximum speed of this driver.
    CLaneDriver(char const * pName, char const * pDescription, float MaxSpeed, float MinSpeed);

    /// @brief Setups the track for this driver.
    /// @param pTrack        Is the track.
    /// @param pCarHandle    Is the car-handle for that driver.
    /// @param ppCarArgument Allows to define an argument-object for the car.
    /// @param pRace         Is the current race situation.
    void setupTrack(tTrack * pTrack, void * pCarHandle, void ** pCarArguments, tSituation * pRace);

    /// @brief Setups a new race for this driver.
    /// @param pCar       Is the car-object of this driver.
    /// @param pSituation Is the situation description for the race.
    void setupRace(tCarElt * pCar, tSituation * pSituation);

    /// @brief Drives the driver.
    /// @param pSituation Is the situation description for the race.
    void drive(tSituation * pSituation);

    /// @brief Ends a race.
    /// @param pSituation Is the situation description for the race.
    void endRace(tSituation * pSituation);

    /// @return Returns the name of the driver.
    char const * getName() const { return mName.c_str(); }

    /// @return Returns the name of the driver.
    char const * getDescription() const { return mDescription.c_str(); }

  private:
    void doFirstStep(tSituation * pSituation);
    void doSpeedControl();
    void doStayInLane();
    void doBackwardTurn();
    int calcGear();
    bool isStuck();
    void choseSpeed(int StartPosition, int Drivers);
    void choseLane();
    void choseStartDelay(int StartPosition);

    std::string mName;
    std::string mDescription;

    tTrack * mpTrack;
    tCarElt * mpCar;

    bool  mIsFirstDriveStep;
    float mMaxSpeed;
    float mMinSpeed;
    float mDriverSpeed;
    int   mStuckCount;
    float mLaneOffset;
    float mStartDelay;
};


#endif //SPEED_DREAMS_2_CLANEDRIVER_H
