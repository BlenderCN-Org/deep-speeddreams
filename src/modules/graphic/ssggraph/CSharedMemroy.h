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

typedef struct
{
  uint32_t ImageWidth;
  uint32_t ImageHeight;
  uint8_t Data[RECORD_MAX_IMAGE_WIDTH * RECORD_MAX_IMAGE_HEIGHT * RECORD_IMAGE_CHANNELS];
} Image_t;

typedef struct
{
  uint8_t IsPauseOn;
  uint8_t IsWritten;
} Sync_t;

typedef struct
{
  Sync_t Sync;
  Image_t Image;
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
};

#endif //SPEED_DREAMS_2_CSHAREDMEMROY_H
