/*************************************************************************************
Phyzaks
Copyright (c) 2012 John Rohrssen
Email: johnrohrssen@gmail.com
*************************************************************************************/

#include <string.h>

#include "config.h"
#include "util.h"
#include "math.h"
#include "game.h"
#include "system.h"
#include "render.h"
#include "physics.h"
#include <stdio.h>
#include <stdlib.h>


GameState game_state()
{
  return s_game.gameState;
}

void game_set_state( GameState state )
{
  s_game.gameState = state;
}

int game_create_object(int assetid)
{
  if( assetid > 0 )
  {
    //assetmanager_load( assetid );
  }

  int object = ++(s_game.aObjects.iTail);

  s_game.aObjects.iAssetId[object] = assetid;

  // render object is game object for now
  s_game.aObjects.uRenderObject[object] = 0;
  s_game.aObjects.uPhysicsObject[object] = 0;
  s_game.aObjects.iLoadState[object] = GameObject::LS_ASSET_LOAD;
  s_game.aObjects.mInitialData[object].bApplied = false;

  return object;
}

int game_object_set_transform( int id, Matrix* pMat)
{
  PROFILE_FUNC();
  int render = s_game.aObjects.uRenderObject[id];
  //render_object_set_transform( render, pMat);

  int physics = s_game.aObjects.uPhysicsObject[id];
  physics_object_set_transform(physics, pMat );

  return 1;
}

int game_object_update_transform( int id, Matrix* pMat)
{
  PROFILE_FUNC();
  int render = s_game.aObjects.uRenderObject[id];
  //render_object_set_transform( render, pMat);

  return 1;
}

int game_object_get_transform( int id, Matrix* pMat)
{
  int physics = s_game.aObjects.uPhysicsObject[id];
  if( physics != 0 )
  {
    physics_object_get_transform( physics, pMat);
    return 1;
  }

  int render = s_game.aObjects.uRenderObject[id];
  if( render != 0 )
  {
    //render_object_get_transform( render, pMat);
    return 1;
  }

  return 0;
}

