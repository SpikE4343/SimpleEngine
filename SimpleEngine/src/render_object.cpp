/*************************************************************************************
Phyzaks
Copyright (c) 2012 John Rohrssen
Email: johnrohrssen@gmail.com
*************************************************************************************/

#include "config.h"
#include "util.h"
#include "game_block.h"
#include "math_game.h"
#include "render.h"
#include "system.h"
#include "asset.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int render_clone_renderobject(int existing, Vector4 pos)
{
  // to many objects in the block
  if( s_renderer.aRenderBlock.iTail - 1 >= OBJECT_BLOCK_SIZE )
    return -1;

  int index = s_renderer.aRenderBlock.iTail++;

  RENDER_OBJ( index, uFlags )   = 1;
  RENDER_OBJ( index, uVisible ) = 0;
  RENDER_OBJ( index, fRadius )  = 1.0f;

  RENDER_OBJ( index, aWorldMatrix ) = matrix_identity();

  RENDER_OBJ( index, uShader ) = RENDER_OBJ( existing, uShader );
  RENDER_OBJ( index, uMesh )   = RENDER_OBJ( existing, uMesh );

  RENDER_OBJ( index, aWorldMatrix ).m[3] = pos;
  RENDER_OBJ( index, iAssetId ) = RENDER_OBJ( existing, iAssetId );
  return index;
}

int render_create_object(int assetid)
{
  //int index = -1;
  if( assetid > 0 )
  {
    assetmanager_load( assetid );
  }

  // to many objects in the block
  if( s_renderer.aRenderBlock.iTail - 1 >= OBJECT_BLOCK_SIZE )
    return -1;

  //if( index == -1 )

  int	index = s_renderer.aRenderBlock.iTail++;

  s_renderer.aRenderBlock.uFlags[index] = 1;
  s_renderer.aRenderBlock.uVisible[index] = 0;
  s_renderer.aRenderBlock.fRadius[index] = 1.0f;

  s_renderer.aRenderBlock.aWorldMatrix[index] = Matrix::IDENTITY;
  s_renderer.aRenderBlock.iAssetId[index] = assetid;

  return index;
}

int render_destroy_object(int object)
{
  if( s_renderer.aRenderBlock.iAssetId[object] == -1 )
    return 0;

  AssetLoadState state = ALS_NONE;

  while( state != ALS_UNLOADED )
  {
    state = assetmanager_unload(s_renderer.aRenderBlock.iAssetId[object]);
    render_object_loaded( object );

    system_sleep(100);
  }

  s_renderer.aRenderBlock.uFlags[object] = 0;
  s_renderer.aRenderBlock.uVisible[object] = 0;
  s_renderer.aRenderBlock.fRadius[object] = 0.0f;

  s_renderer.aRenderBlock.aWorldMatrix[object] = Matrix::IDENTITY;
  s_renderer.aRenderBlock.iAssetId[object] = -1;
  return 0;
}

void render_object_set_transform(int id, Matrix* pMat)
{
  if( id >= 0 && id < s_renderer.aRenderBlock.iTail )
    s_renderer.aRenderBlock.aWorldMatrix[id] = *pMat;
}

void render_object_get_transform(int id, Matrix* pMat)
{
  if( id >= 0 && id < s_renderer.aRenderBlock.iTail )
    *pMat = s_renderer.aRenderBlock.aWorldMatrix[id];
}

int render_object_loaded(int iObj)
{
  return 0;
  //if( s_renderer.iProcessedAssets >= MAX_RENDERER_ASSET_LOADS )
  //  return 0;

  //int assetid = s_renderer.aRenderBlock.iAssetId[iObj];
  //if( assetid == -1 )
  //{
  //  // no asset for this object to render
  //  return 0;
  //}

  //int ready = 1;
  //int mesh = s_renderer.aRenderBlock.uMesh[iObj];
  //int shader = s_renderer.aRenderBlock.uShader[iObj];
  //AssetLoadState load_state = asset_load_state(assetid);

  //switch( load_state )
  //{

  //case ALS_READY:
  //  if( shader != 0 && mesh != 0 )
  //  {
  //    if( mesh != 0 )
  //      ready &= render_mesh_loaded( mesh );

  //    if( shader != 0 )
  //      ready &= render_shader_loaded( shader );

  //    break;
  //  }

  //case ALS_LOADED:
  //  {
  //    // asset for this object has been loaded into memory
  //    // and needs to be processed by this renderer

  //    // make sure all dependent assets are loaded
  //    Asset* pAsset = assetmanager_get_data( assetid );
  //    if( pAsset == NULL )
  //      break;

  //    if( pAsset->pMainBlock->u32FourCC != AT_RENDER_OBJECT )
  //      break;

  //    for( int block=0; block < pAsset->u32NumDataBlocks; ++block )
  //    {
  //      AssetBlock* pBlock = pAsset->pDataBlock[block];
  //      switch( pBlock->u32FourCC )
  //      {
  //      case AT_RENDER_OBJECT_INFO:
  //        {
  //          RenderObjectInfoAssetBlock* pGMI = (RenderObjectInfoAssetBlock*)pBlock;
  //          s_renderer.aRenderBlock.fRadius[iObj] = (pGMI->bounds_max[1] - pGMI->bounds_min[1])*0.5f;
  //        }
  //        break;

  //      case AT_RENDER_OBJECT_MESH:
  //        {
  //          int mesh_assetid = *(int*)asset_block_get_data((AssetBlock*)pBlock);
  //          int meshid = render_create_mesh(mesh_assetid);	

  //          s_renderer.aMeshBlock.iAssetId[meshid] = mesh_assetid;
  //          s_renderer.aRenderBlock.uMesh[iObj] = meshid;

  //          int shader_assetid = *(int*)(asset_block_get_data((AssetBlock*)pBlock)+sizeof(int));
  //          debug_log( "shader_assetid: %d", shader_assetid );
  //          int shaderid = render_create_shader(shader_assetid);

  //          s_renderer.aShaderBlock.aShaders[shaderid].iAssetId = shader_assetid;
  //          s_renderer.aRenderBlock.uShader[iObj] = shaderid;
  //        }
  //        break;
  //      }
  //    }

  //    asset_set_load_state( assetid, ALS_READY );

  //    s_renderer.iProcessedAssets++;
  //    ready = 0;
  //  }
  //  break;

  //case ALS_UNLOAD:
  //case ALS_UNLOAD_RELOAD:
  //  {
  //    int mesh = s_renderer.aRenderBlock.uMesh[iObj];
  //    int shader = s_renderer.aRenderBlock.uShader[iObj];

  //    if( assetmanager_unload(s_renderer.aMeshBlock.iAssetId[mesh]) == ALS_UNLOADED &&
  //      assetmanager_unload(s_renderer.aShaderBlock.aShaders[shader].iAssetId) == ALS_UNLOADED)
  //    {
  //      asset_set_load_state( assetid, load_state == ALS_UNLOAD_RELOAD ? ALS_UNLOADED_RELOAD : ALS_UNLOADED );
  //    }

  //    s_renderer.aRenderBlock.uMesh[iObj] = 0;
  //    s_renderer.aRenderBlock.uShader[iObj] = 0;
  //    ready = 0;
  //  }
  //  break;

  //default:
  //  ready = 0;
  //  break;
  //}

  //return ready;
}