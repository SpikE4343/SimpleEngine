/*************************************************************************************
Phyzaks
Copyright (c) 2012 John Rohrssen
Email: johnrohrssen@gmail.com
*************************************************************************************/

#include "config.h"
#include "util.h"
#include "math_game.h"
#include "render.h"
#include "system.h"
#include "physics.h"
#include "game.h"
#include "asset.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#if USE_NULL_PHYSICS

int physics_initialize()
{
  PROFILE_FUNC();
  return 1;
}

int physics_update(float fDt)
{
  PROFILE_FUNC();
  return 1;
}

int physics_shutdown()
{
  return 1;
}

void physics_set_timemult( float fTimeMult)
{
}

void physics_set_gravity( const Vector4& vGravity)
{
}

int physics_create_object(int ownerId, int assetid)
{
  return 1;
}

int physics_object_loaded( int iObj )
{
  PROFILE_FUNC();
  return 1;
}

int physics_object_get_transform( int id, Matrix* pMat)
{
  return 1;
}

int physics_object_set_transform( int id, Matrix* pMat)
{
  PROFILE_FUNC();
  return 1;
}

#endif
