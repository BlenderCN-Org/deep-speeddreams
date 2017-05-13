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
 * @file CSharedMemroy.cpp
 * @author Andre Netzeband
 * @date 12.05.2017
 *
 * @brief TODO
 *
 */

#include "CSharedMemroy.h"

#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::interprocess;

typedef windows_shared_memory Memory_t;
typedef mapped_region Region_t;

CSharedMemory::CSharedMemory():
  mpMemory(NULL),
  mpRegion(NULL)
{
  mpMemory = new Memory_t(open_or_create, "DeepDrivingMemory", read_write, sizeof(Data_t));
  assert(mpMemory);

  mpRegion = new Region_t(*mpMemory, read_write);
  assert(mpRegion);

  memset(getAddress(), 0, sizeof(Data_t));
}

CSharedMemory::~CSharedMemory()
{
  if (mpRegion)
  {
    delete(mpRegion);
    mpRegion = NULL;
  }

  if (mpMemory)
  {
    delete(mpMemory);
    mpMemory = NULL;
  }
}

Data_t * CSharedMemory::getAddress()
{
  return (Data_t*)mpRegion->get_address();
}


void CSharedMemory::waitOnRead()
{
  while (getAddress()->Sync.IsWritten && getAddress()->Sync.IsPauseOn)
  {
    boost::this_thread::sleep(boost::posix_time::microseconds(1));
  }
}


void CSharedMemory::indicateWrite()
{
  getAddress()->Sync.IsWritten = 1;
}