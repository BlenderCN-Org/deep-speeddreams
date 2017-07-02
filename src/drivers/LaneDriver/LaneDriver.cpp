/***************************************************************************

    file                 : LaneDriver1.cpp
    created              : 2014年 09月 01日 星期一 13:08:18 EDT
    copyright            : (C) 2002 Chenyi Chen
    copyright            : (C) 2017 Andre Netzeband

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This file was originally created from Chenyi Chen for the Deep-       *
 *   Driving project.                                                      *
 *                                                                         *
 ***************************************************************************/


#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <math.h>

#include <tgf.h> 
#include <track.h> 
#include <car.h> 
#include <raceman.h> 
#include <robot.h>

#include "CLaneDriver.h"

static tTrack	*curTrack;

static float const sMaxSpeed = 43;
static float const sMinSpeed = 25;
static int const sNumberOfDrivers = 30;
static CLaneDriver * spDriver[sNumberOfDrivers];

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s); 
static void newrace(int index, tCarElt* car, tSituation *s); 
static void drive(int index, tCarElt* car, tSituation *s); 
static void endrace(int index, tCarElt *car, tSituation *s);
static void shutdown(int index);
static int  InitFuncPt(int index, void *pt); 

static char * duplicateString(char const * const pString)
{
  size_t const Length = strlen(pString);
  char * const pNewString = new char[Length+1];
  strcpy(pNewString, pString);

  assert(pNewString[Length] == '\0');

  return pNewString;
}

/*
 * Module entry point  
 */ 
extern "C" int LaneDriver(tModInfo *modInfo)
{
  memset(modInfo, 0, MAX_MOD_ITF*sizeof(tModInfo));

  std::string Description = "Drives just on a single lane.";

  for(int i = 0; (i < MAX_MOD_ITF) && (i < sNumberOfDrivers); i++)
  {
    std::string const Name = "Lane Driver " + std::to_string(i+1);

    assert(spDriver[i] == NULL);
    spDriver[i] = new CLaneDriver(Name.c_str(), Description.c_str(), sMaxSpeed, sMinSpeed);
    assert(spDriver[i] != NULL);

    modInfo[i].name    = duplicateString(spDriver[i]->getName());	       /* name of the module (short) */
    modInfo[i].desc    = duplicateString(spDriver[i]->getDescription());	 /* description of the module (can be long) */
    modInfo[i].fctInit = InitFuncPt;		                                   /* init function */
    modInfo[i].gfId    = ROB_IDENT;		                                     /* supported framework version */
    modInfo[i].index   = i+1;
  }

  return 0;
} 

/* Module interface initialization. */
static int InitFuncPt(int index, void *pt)
{ 
    tRobotItf *itf  = (tRobotItf *)pt; 

    itf->rbNewTrack = initTrack;  /* Give the robot the track view called */
				                          /* for every track change or new race */
    itf->rbNewRace  = newrace; 	  /* Start a new race */
    itf->rbDrive    = drive;	    /* Drive during race */
    itf->rbPitCmd   = NULL;
    itf->rbEndRace  = endrace;	  /* End of the current race */
    itf->rbShutdown = shutdown;	  /* Called before the module is unloaded */
    itf->index      = index; 	    /* Index used if multiple interfaces */
    return 0; 
} 

/* Called for every track change or new race. */
static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s)
{
  int const i = index-1;
  assert(i >= 0);

  curTrack = track;
  *carParmHandle = NULL;

  assert(spDriver[i] != NULL);
  spDriver[i]->setupTrack(track, carHandle, carParmHandle, s);
}

/* Start a new race. */
static void newrace(int index, tCarElt* car, tSituation *s)
{
  int const i = index-1;
  assert(i >= 0);

  assert(spDriver[i] != NULL);
  spDriver[i]->setupRace(car, s);
} 

static void drive(int index, tCarElt* car, tSituation *s) 
{
  int const i = index-1;
  assert(i >= 0);

  assert(spDriver[i] != NULL);
  spDriver[i]->drive(s);
}

/* End of the current race */
static void endrace(int index, tCarElt *car, tSituation *s)
{
  int const i = index - 1;
  assert(i >= 0);

  assert(spDriver[i] != NULL);
  spDriver[i]->endRace(s);
}

/* Called before the module is unloaded */
static void shutdown(int index)
{
  int const i = index - 1;
  assert(i >= 0);

  assert(spDriver[i] != NULL);
  delete(spDriver[i]);
}

