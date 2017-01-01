/*************************************************************************************
Phyzaks
Copyright (c) 2012 John Rohrssen
Email: johnrohrssen@gmail.com
*************************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "util.h"
#include "math.h"
#include "game.h"
#include "system.h"
#include "render.h"
#include "physics.h"


struct PhyszaksGame : GameData
{
  GameObjectBlock aObjects;
  int iClientId;
  Vector4 vGravity;
};

PhyszaksGame s_game_data;
GameData& game_data()
{
  return s_game_data;
}

int game_begin_initialize()
{
  if( s_game.gameState != GS_INITIAL)
    return 1;

  s_game.gameState = GS_BEGIN_INITIALIZING;

  return 1;
}

int game_object_loaded( int iObj );

int game_initialize()
{
  int error = 0;

  system_log( "game initialize" );

  if( s_game.gameState != GS_BEGIN_INITIALIZING)
    return 1;

  s_game.gameState = GS_LOADING;

  /*error = assetmanager_initialize();
  if( error != 1 )
  {
  system_log( "assetmanager_initialize failed: %d", error);
  return 0;
  }*/

  int width = 640;
  int height = 480;

  error = system_createWindow( width, height, "SimpleEngine" );
  if( error != 1 )
  {
    system_log( "system_createWindow failed: %d", error);
    return 0;
  }

  error = render_initialize(width,height);
  if( error != 1 )
  {
    system_log( "render_initialize failed: %d", error);
    return 0;
  }

  error = physics_initialize();
  if( error != 1)
  {
    system_log( "physics_initialize failed: %d", error);
    return 0;
  }

  //assetmanager_add_clump( "phyzaks.clump" );

  return 1;
}

int game_load_level( int level_assetid )
{
  //s_game.iLevelAssetId = assetmanager_load(level_assetid);

  return 1;
}

int game_level_loaded( int assetid )
{
  if( assetid == -1 )
  {
    // no asset for this object to render
    return 0;
  }

  int ready = 1;

  //AssetLoadState load_state = asset_load_state(assetid);

  //switch( load_state )
  //{
  //case ALS_LOADED:
  //  {
  //    // asset for this object has been loaded into memory
  //    // and needs to be processed by this renderer

  //    // make sure all dependent assets are loaded
  //    Asset* pAsset = assetmanager_get_data( assetid );
  //    if( pAsset == NULL )
  //      break;

  //    if( pAsset->pMainBlock->u32FourCC != AT_GAME_LEVEL)
  //      break;

  //    {
  //      AssetBlock* pBlock = pAsset->pDataBlock[pAsset->u32NextLoadBlock];

  //      switch( pBlock->u32FourCC )
  //      {
  //      case AT_GAME_LEVEL_INFO:
  //        {
  //          GameLevelInfoAssetBlock* pGLI = (GameLevelInfoAssetBlock*)pBlock;
  //          vector4_set(&s_game.vGravity, pGLI->gravity[0], pGLI->gravity[1], pGLI->gravity[2], 0.0f);
  //          physics_set_gravity( s_game.vGravity ); 
  //        }
  //        break;

  //      case AT_GAME_LEVEL_OBJECT:
  //        {


  //          GameLevelObjectAssetBlock* pGLO = (GameLevelObjectAssetBlock*)pBlock;

  //          int game_object = game_create_object(pGLO->iAssetId );
  //          Matrix mat;
  //          matrix_set(&mat, &pGLO->world_matrix[0]);
  //          s_game.aObjects.mInitialData[game_object].mInitialTransform = mat;
  //        }
  //        break;
  //      }
  //    }

  //    if( ++pAsset->u32NextLoadBlock >= pAsset->u32NumDataBlocks )
  //    {
  //      asset_set_load_state( assetid, ALS_READY );
  //    }
  //    ready = 0;
  //  }
  //  break;

  //case ALS_READY:
  //  {
  //    for( int i=1; i < s_game.aObjects.iTail; ++i )
  //    {
  //      ready &= game_object_loaded(i);
  //    }
  //  }
  //  break;

  //case ALS_UNLOAD:
  //case ALS_UNLOAD_RELOAD:
  //  {
  //    ready = 0;
  //  }
  //  break;

  //default:
  //  ready = 0;
  //  break;
  //}

  return ready;
}

int game_update_ui( float fDt )
{
  PROFILE_FUNC();
  return 1;
}

int game_draw_ui( float fDt )
{
  PROFILE_FUNC();
  //render_draw_text( 1, "hello world", 0, s_renderer.iHeight - s_renderer.aFontBlock.aFonts[1].iCharHeight );
  return 1;
}

int game_object_loaded( int iObj )
{
  PROFILE_FUNC();
  int ready = 1;

  int loadstate = s_game.aObjects.iLoadState[iObj];
  int physics   = s_game.aObjects.uPhysicsObject[iObj];
  int render    = s_game.aObjects.uRenderObject[iObj];

  //switch( loadstate )
  //{
  //case GameObject::LS_ASSET_LOAD:
  //  {
  //    int assetid = s_game.aObjects.iAssetId[iObj];
  //    if( assetid == -1 )
  //    {
  //      // no asset for this object to render
  //      return 0;
  //    }

  //    AssetLoadState load_state = asset_load_state(assetid);

  //    switch( load_state )
  //    {
  //    case ALS_LOADED:
  //      {
  //        // asset for this object has been loaded into memory
  //        // make sure all dependent assets are loaded
  //        Asset* pAsset = assetmanager_get_data( assetid );
  //        if( pAsset == NULL )
  //          break;

  //        if( pAsset->pMainBlock->u32FourCC != AT_GAME_MODEL )
  //          break;

  //        for( int block=0; block < pAsset->u32NumDataBlocks; ++block )
  //        {
  //          AssetBlock* pBlock = pAsset->pDataBlock[block];

  //          switch( pBlock->u32FourCC )
  //          {
  //          case AT_GAME_MODEL_INFO:
  //            break;
  //          case AT_GAME_MODEL_RENDER:
  //            {
  //              int render_assetid = *(int*)asset_block_get_data((AssetBlock*)pBlock);
  //              s_game.aObjects.uRenderObject[iObj] = render_create_object(render_assetid);
  //            }
  //            break;

  //          case AT_GAME_MODEL_PHYSICS:
  //            {
  //              int physics_assetid = *(int*)asset_block_get_data((AssetBlock*)pBlock);
  //              s_game.aObjects.uPhysicsObject[iObj] = physics_create_object(iObj, physics_assetid );
  //            }
  //            break;

  //          }
  //        }

  //        ready = 0;

  //        s_game.aObjects.iLoadState[iObj] = GameObject::LS_LOADED;
  //      }
  //      break;

  //    case ALS_UNLOAD:
  //    case ALS_UNLOAD_RELOAD:
  //      {
  //        int physics = s_game.aObjects.uPhysicsObject[iObj];
  //        int render = s_game.aObjects.uRenderObject[iObj];

  //        s_game.aObjects.uPhysicsObject[iObj] = 0;
  //        s_game.aObjects.uRenderObject[iObj] = 0;
  //        ready = 0;
  //      }
  //      break;

  //    default:
  //      ready = 0;
  //      break;
  //    }
  //  }
  //  break;

  //case GameObject::LS_LOADED:
  //  ready &= physics_object_loaded( physics );
  //  ready &= render_object_loaded( render );

  //  GameObjectInitialData& init_data = s_game.aObjects.mInitialData[iObj];
  //  if( ready && !init_data.bApplied )
  //  {
  //    game_object_set_transform( iObj, &init_data.mInitialTransform);

  //    s_game.aObjects.iLoadState[iObj] = GameObject::LS_READY;

  //    init_data.bApplied = true;
  //  }
  //  break;
  //}

  return ready;
}

int game_update_object(int iObj, float fDt )
{
  PROFILE_FUNC();
  if( !game_object_loaded(iObj) )
    return 0;

  return 1;
}

int game_update_objects(float fDt)
{	
  //PROFILE_FUNC();
  float angle = 0.1f*fDt;
  Matrix rot,tempMat;
  matrix_rotation_y(&rot, angle);
  //matrix_rotation_x(&rot, angle);
  //matrix_rotation_y(&tempMat, -angle);
  //multiply( &rot, &tempMat );
  //matrix_rotation_z(&tempMat, angle);
  //multiply( &rot, &tempMat );

  PROFILE_BLOCK( ObjectUpdate );
  RendererData& data = render_data();
  Vector4 pos;
  for( int i=1;i <= s_game.aObjects.iTail; ++i )
  {
    game_update_object( i, fDt );
  }

  return 1;
}

//int game_process_message( PendingMessage* pMsg )
//{
//  switch( pMsg->iEvent )
//  {
//  case PNM_INVALID:
//    break;
//  case PNM_CONNECT:
//    break;
//  case PNM_CONNECT_REPLY:
//    {
//      PhyzaksConnectReplyMessage* msg = (PhyzaksConnectReplyMessage*)pMsg->aBuffer;
//      s_game_data.iClientId = msg->iClientId;
//      game_set_state( GS_CLIENT_READY );
//    }
//    break;
//  case PNM_LOAD_LEVEL:
//    {
//      PhyzaksLoadLevelMessage* msg = (PhyzaksLoadLevelMessage*)pMsg->aBuffer;
//      debug_log("Loading server level %d", msg->iAssetId );
//      s_game_data.iLevelAssetId = msg->iAssetId;
//      game_set_state( GS_LOADING );
//    }
//    break;
//  case PNM_GAME_STATE:
//    break;
//  case PNM_GAME_CLIENT_STATE:
//    break;
//  case PNM_DISCONNECT:
//    break;
//  }
//
//  return 1;
//}

//int game_process_event( PendingMessage* pMsg )
//{
//  switch( pMsg->iEvent )
//  {
//  case CE_CONNECTED:
//    // connected to server
//    game_set_state( GS_CLIENT_CONNECTED );
//    break;
//
//  case CE_DISCONNECTED:
//    // disconnected from server
//    break;
//
//  case CE_MESSAGE:
//    game_process_message( pMsg );
//    break;
//  }
//
//  return 1;
//}

//int game_update_network(float fDt, float maxT )
//{
//  client_update( fDt );
//
//  int count = client_message_count();
//  int i = count;
//
//  while( i > 0 )
//  {
//    PendingMessage* pMsg = client_get_message(i);
//    --i;
//
//    if( pMsg == NULL )
//      continue;
//
//    // TODO: check total time processing and stop if we have taken too long
//  }
//
//  if( i == 0 )
//  {
//    client_messages_processed();
//  }
//
//  return 1;
//}

char debugText[512];
int game_update()
{
  PROFILE_FUNC();
  if( system_update() == 0 )
    return 0;

  if( system_isKeyDown( Input::K_ESCAPE ) )
    return 0;

  float fDt = system_get_frame_time_secs();

  //system_log( "%.2f s, %.2f, objs: %d/%d", fDt, fDt*1000, render_data().iVisibleObjects, render_data().aRenderBlock.iTail);

  switch( s_game.gameState )
  {
  case GS_INITIAL:
    // wait for the renderer/system to fully initialize
    s_game.gameState = GS_BEGIN_INITIALIZING;
    return 1;

  case GS_BEGIN_INITIALIZING:
    {
      //if( system_mouse_y() < 100 )
      {
        game_initialize();
      }
      //else
      //{
      //	return 1;
      //}

      s_game.gameState = GS_RUNNING;
    }
    break;


  case GS_LOADING:
    {
      int ready = game_level_loaded( s_game.iLevelAssetId );
      if( ready != 0 )
      {
        s_game.gameState = GS_RUNNING;
      }
    }
    break;

  case GS_RUNNING:
    {
      physics_set_gravity( s_game.vGravity );

      // update physics data
      physics_update(fDt);

      float fSpeed = 10.0f * fDt;

      Matrix* pCameraMat = render_view_matrix();
      Matrix tempMat, tempCameraMat = *pCameraMat;

      Vector4 vOffsetFinal=Vector4::Zero, vOffsetAxis, vOldTrans;

      //vOldTrans = pCameraMat->m[3];
      //pCameraMat->m[3] = Vector4::ZERO;

      //if( system_mouse_button_down( Input::RIGHT_BUTTON ) )
      {
        tempCameraMat = *pCameraMat;

        //tempCameraMat.m[0].y = 0;
        //cross( &tempCameraMat.m[1], &tempCameraMat.m[0], &tempCameraMat.m[2] );
        //vector4_normalize( &pCameraMat->m[1] );

        matrix_rotation_y( &tempMat, (float)system_mouse_delta_x()*0.01f );
        multiply( &tempCameraMat, &tempMat );

        matrix_rotation_x( &tempMat, (float)system_mouse_delta_y()*0.01f );
        multiply( &tempCameraMat, &tempMat );

        //tempCameraMat.m[3] = pCameraMat->m[3];
        *pCameraMat = tempCameraMat;
      }

      matrix_rotation_z( &tempMat, fSpeed * ( system_isKeyDown( Input::K_E ) - system_isKeyDown( Input::K_Q )) );
      multiply( pCameraMat, &tempMat );

      pCameraMat->m[3].x += fSpeed * (system_isKeyDown( Input::K_A ) - system_isKeyDown( Input::K_D ));
      pCameraMat->m[3].y += fSpeed * (system_isKeyDown( Input::K_X ) - system_isKeyDown( Input::K_SPACE ));
      pCameraMat->m[3].z += fSpeed * (system_isKeyDown( Input::K_W ) - system_isKeyDown( Input::K_S ));

      //translate( pCameraMat, vOffsetFinal);

      //matrix_rotation_z( &tempMat, fSpeed * ( system_isKeyDown( Input::K_E ) - system_isKeyDown( Input::K_Q )) );
      //multiply( pCameraMat, &tempMat );

      /*PhyzaksClientStateMessage msg;
      msg.iClientId = */

      game_update_objects(fDt);
    }
    break;

  case GS_CLOSING:
    break;
  }

  //game_update_network(fDt, 0.001f);

  game_update_ui(fDt);

  render_begin_frame();

  render_draw();

  game_draw_ui(fDt);

  PROFILER_UPDATE();

  render_end_frame();

  return 1;
}

int game_shutdown()
{
  render_shutdown();
  physics_shutdown();

  return 1;
}

int game_run(const char* args) 
{
  s_game.gameState = GS_INITIAL;

  while( game_update() );

  game_shutdown();

  return 1;
}

