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
 * @file CSharedMemroy.h
 * @author Andre Netzeband
 * @date 12.05.2017
 *
 * @brief Encapsulates the shared memory.
 *
 */

#ifndef SPEED_DREAMS_2_CSHAREDMEMROY_H
#define SPEED_DREAMS_2_CSHAREDMEMROY_H

#include <cstdint>

#if defined(_WIN32)
#include <boost/interprocess/windows_shared_memory.hpp>
#endif

#include <boost/interprocess/mapped_region.hpp>

#if defined(_WIN32)
typedef boost::interprocess::windows_shared_memory Memory_t;
#endif
typedef boost::interprocess::mapped_region Region_t;

#define RECORD_MAX_IMAGE_WIDTH 2560
#define RECORD_MAX_IMAGE_HEIGHT 1440
#define RECORD_IMAGE_CHANNELS 3
#define MAX_TRACK_NAME_LENGTH 256

/// @brief Contains the image.
typedef struct
{
  /// @brief The real width of the recorded image.
  uint32_t ImageWidth;

  /// @brief The real height of the recorded image.
  uint32_t ImageHeight;

  /// @brief The memory reserved for the image itself.
  uint8_t Data[RECORD_MAX_IMAGE_WIDTH * RECORD_MAX_IMAGE_HEIGHT * RECORD_IMAGE_CHANNELS];
} Image_t;


/// @brief Contains flags and values used for synchronization.
typedef struct
{
  /// @brief Indicates if Speed-Dream should wait until "IsWritten" is set to 0, before new values are written.
  uint8_t IsPauseOn;

  /// @brief Is set to 1 as soon new values have been written from Speed-Dreams.
  uint8_t IsWritten;

  /// @brief Contains the number of frames, that have been written to the memory, since the race has been started.
  uint64_t WriteNumber;
} Sync_t;


/// @brief Contains the labels for the deep-driving project.
typedef struct
{
  /// @brief The relative angle between the car and the middle line of the street.
  ///        The value is given in arc and is always in an interval of -pi to +pi.
  float Angle;

  /// @brief Indicates if the street is straight for at least 60m or if a curve is near than 60m.
  ///        This value is eighter 0 or 1.
  float Fast;

  /// @brief The distance between the mid of the car to the left lane marking of the left lane. (Only valid, if the car is inside a lane.)
  ///        The value is given in meter. The valid range is -7.5 to -4.5. If the system is not active or no left lane exists, the value is set to -9.
  float LL;

  /// @brief The distance between the mid of the car to the left lane marking of the current lane. (Only valid if the car is inside a lane.)
  ///        The value is given in meter. The valid range is -3.5 to -0.5. If the system is not active, the value is set to -5.
  float ML;

  /// @brief The distance between the mid of the car to the right lane marking of the current lane. (Only valid if the car is inside a lane.)
  ///        The value is given in meter. The valid range is 0.5 to 3.5. If the system is not active, the value is set to 5.
  float MR;

  /// @brief The distance between the mid of the car to the right lane marking of the right lane. (Only valid if the car is inside a lane.)
  ///        The value is given in meter. The valid range is 4.5 to 7.5. If the system is not active or no right lane exists, the value is set to 9.
  float RR;

  /// @brief The distance between the mid of the car to a traffic car which is driving on the left lane. (Only valid if the car is inside a lane.)
  ///        The value is given in meter. The valid range is 0 to 60. If the system is not active or if no traffic car on the left lane is available,
  ///        the value is set to 90.
  float DistLL;

  /// @brief The distance between the mid of the car to a traffic car which is driving on the current lame. (Only valid if the car is inside a lane.)
  ///        The value is given in meter. The valid range is 0 to 60. If the system is not active or if no traffic car on the current lane is available,
  ///        the value is set to 90.
  float DistMM;

  /// @brief The distance between the mid of the car to a traffic car which is driving on the right lane. (Only valid if the car is inside a lane.)
  ///        The value is given in meter. The valid range is 0 to 60. If the system is not active or if no traffic car on the right lane is available,
  ///        the value is set to 90.
  float DistRR;

  /// @brief The distance between the mid of the car to the left lane marking. (Only valid, if the car is driving between two lanes - on the marking.)
  ///        The value is given in meter. The valid range is -5 to -3. If the system is not active or no left lane exists, the value is set to -7.
  float L;

  /// @brief The distance between the mid of the car to the middle lane marking. (Only valid if the car is driving between two lanes - on the marking.)
  ///        The value is given in meter. The valid range is -2 to 2. If the system is not active, the value is set to -5.
  float M;

  /// @brief The distance between the mid of the car to the right lane marking. (Only valid if the car is driving between two lanes - on the marking.)
  ///        The value is given in meter. The valid range is 3 to 5. If the system is not active or no right lane exists, the value is set to 7.
  float R;

  /// @brief The distance between the mid of the car to a traffic car which is driving on the left lane. (Only valid if the car is driving between two lanes - on the marking.)
  ///        The value is given in meter. The valid range is 0 to 60. If the system is not active or if no traffic car on the left lane is available,
  ///        the value is set to 90.
  float DistL;

  /// @brief The distance between the mid of the car to a traffic car which is driving on the right lane. (Only valid if the car is driving between two lanes - on the marking.)
  ///        The value is given in meter. The valid range is 0 to 60. If the system is not active or if no traffic car on the right lane is available,
  ///        the value is set to 90.
  float DistR;
} Labels_t;


/// @brief Additional information about the game situation.
typedef struct
{
  /// @brief The current speed of the car. The value is given in km/h.
  float Speed;

  /// @brief The number of lanes for the current track. This value is 0, if the number of lanes is unknown.
  uint8_t Lanes;

  /// @brief The name of the current track. If the name is too long (longer than 256 characters, the name is truncated).
  char TrackName[MAX_TRACK_NAME_LENGTH];

  /// @brief An unique ID for the track.
  uint64_t UniqueTrackID;

  /// @brief An unique ID for the race.
  uint64_t UniqueRaceID;
} Game_t;


/// @brief Driving commands for the car.
typedef struct
{
  /// @brief Indicates if the car is driven by the game (0) or by the network (1).
  uint8_t IsControlling;

  /// @brief The steering command.
  float Steering;

  /// @brief The accelerating command.
  float Accelerating;

  /// @brief The breaking command.
  float Breaking;
} Control_t;


/// @brief The overall memory structure.
typedef struct
{
  /// @brief Information used to sync reader and writer process.
  Sync_t Sync;

  /// @brief Contains the image.
  Image_t Image;

  /// @brief The labels used by deep-driving.
  Labels_t Labels;

  /// @brief Additional information about the game situation.
  Game_t Game;

  /// @brief Driving commands for the car.
  Control_t Control;
} Data_t;


class CSharedMemory
{
  public:
    CSharedMemory();
    ~CSharedMemory();

    Data_t * getAddress();
    void indicateWrite();
    void waitOnRead();

  protected:
    Memory_t * mpMemory;
    Region_t * mpRegion;
    uint64_t   mWriteNumber;
};

#endif //SPEED_DREAMS_2_CSHAREDMEMROY_H
