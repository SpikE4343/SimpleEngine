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
#include "asset/asset.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int render_create_mesh(int assetid)
{
	int index = -1;
	if( assetid > 0 )
	{
		bool bFound = false;
		for( int i=1; i < s_renderer.aMeshBlock.iTail; ++i )
		{
			if( s_renderer.aMeshBlock.iAssetId[i] == assetid )
			{
				bFound = true;
				index = i;
				break;
			}
		}

		assetmanager_load( assetid );
	}

	// to many objects in the block
	if( index == -1 && s_renderer.aMeshBlock.iTail < RenderObjectBlock::BLOCK_SIZE - 1 )
	{
		index = s_renderer.aMeshBlock.iTail++;

		s_renderer.aMeshBlock.uVertexBuffer[index] = 0;
		s_renderer.aMeshBlock.uIndexBuffer[index]  = 0;
		s_renderer.aMeshBlock.iAssetId[index] = assetid;
	}

	return index;
}

int render_mesh_loaded(int iMesh)
{
	if( s_renderer.iProcessedAssets >= MAX_RENDERER_ASSET_LOADS )
		return 0;

	int assetid = s_renderer.aMeshBlock.iAssetId[ iMesh ];
	if( assetid == -1 )
	{
		// no asset for this object to render
		return 0;
	}

	AssetLoadState load_state = asset_load_state(assetid);

	int ready = 1;

	switch( load_state )
	{
	case ALS_LOADED:
		{
			debug_log( "shader asset: %d loaded, render processing", assetid );

			// asset for this object has been loaded into memory
			// and needs to be processed by this renderer

			// make sure all dependent assets are loaded
			Asset* pAsset = assetmanager_get_data( assetid );
			if( pAsset == NULL )
				break;

			if( pAsset->pMainBlock->u32FourCC != AT_RENDER_MESH )
				break;

			{
				AssetBlock* pBlock = pAsset->pDataBlock[pAsset->u32NextLoadBlock];
				switch( pBlock->u32FourCC )
				{
				case AT_RENDER_MESH_VERTEX:
					{
						int count = pBlock->u32Size / (sizeof(float)*3);

						s_renderer.aMeshBlock.uVertexBuffer[iMesh] = render_create_buffer( Render::BT_ARRAY, 
																						  Render::BU_STATIC, 
																						  pBlock->u32Size, 
																						  (void*)asset_block_get_data(pBlock));


						s_renderer.aMeshBlock.uNumVerts[iMesh]     = count;

						debug_log( "vertex buffer %u created", s_renderer.aMeshBlock.uVertexBuffer[iMesh]);
					}
					break;

				case AT_RENDER_MESH_INDEX:
					{
						int count = pBlock->u32Size / (sizeof(unsigned short)*3);
						s_renderer.aMeshBlock.uIndexBuffer[iMesh] = render_create_buffer( Render::BT_ELEMENT_ARRAY, 
																						  Render::BU_STATIC, 
																						  pBlock->u32Size, 
																						  (void*)asset_block_get_data(pBlock));

						debug_log( "UV buffer %u created", s_renderer.aMeshBlock.uIndexBuffer[iMesh]);
						s_renderer.aMeshBlock.uNumIndices[iMesh]  = count*3;
					}
					break;

				case AT_RENDER_MESH_UV:
					{
						int count = pBlock->u32Size / (sizeof(float)*2);
						s_renderer.aMeshBlock.uTexCoordBuffer[iMesh] = render_create_buffer( Render::BT_ARRAY, 
																								 Render::BU_STATIC, 
																								 pBlock->u32Size, 
																								 (void*)asset_block_get_data(pBlock));

						debug_log( "UV buffer %u created", s_renderer.aMeshBlock.uTexCoordBuffer[iMesh]);

						
						s_renderer.aMeshBlock.uNumTexCoords[iMesh]   = count;
					}
					break;

				case AT_RENDER_MESH_VERTEX_NORMAL:
					{
						int count = pBlock->u32Size / (sizeof(float)*3);
						s_renderer.aMeshBlock.uVertexNormalBuffer[iMesh] = render_create_buffer( Render::BT_ARRAY, 
																								 Render::BU_STATIC, 
																								 pBlock->u32Size, 
																								 (void*)asset_block_get_data(pBlock));
						s_renderer.aMeshBlock.uNumVertNormals[iMesh]     = count;

						debug_log( "vertex normal buffer %u created", s_renderer.aMeshBlock.uVertexNormalBuffer[iMesh]);
					}
					break;
				}
			}

			if( ++pAsset->u32NextLoadBlock >= pAsset->u32NumDataBlocks )
			{
				asset_set_load_state( assetid, ALS_READY );
			}
			
			debug_log( "shader asset: %d ready, dependencies pending", assetid );
		}

		ready =0;
		break;

	case ALS_READY:
		{
			ready =1; 
		}
		break;

	case ALS_UNLOAD:
	case ALS_UNLOAD_RELOAD:

		if( s_renderer.aMeshBlock.uIndexBuffer[iMesh] != 0 )
		{
			render_destroy_buffer( s_renderer.aMeshBlock.uIndexBuffer[iMesh] );
			s_renderer.aMeshBlock.uNumIndices[iMesh] = 0;
			s_renderer.aMeshBlock.uIndexBuffer[iMesh] = 0;
		}

		if( s_renderer.aMeshBlock.uTexCoordBuffer[iMesh] != 0 )
		{
			render_destroy_buffer( s_renderer.aMeshBlock.uTexCoordBuffer[iMesh] );
			s_renderer.aMeshBlock.uNumTexCoords[iMesh] = 0;
			s_renderer.aMeshBlock.uTexCoordBuffer[iMesh] = 0;
		}

		if( s_renderer.aMeshBlock.uVertexBuffer[iMesh] != 0 )
		{
			render_destroy_buffer( s_renderer.aMeshBlock.uVertexBuffer[iMesh] );
			s_renderer.aMeshBlock.uNumVerts[iMesh] = 0;
			s_renderer.aMeshBlock.uVertexBuffer[iMesh] = 0;
		}

		if( s_renderer.aMeshBlock.uVertexNormalBuffer[iMesh] != 0 )
		{
			render_destroy_buffer( s_renderer.aMeshBlock.uVertexNormalBuffer[iMesh] );
			s_renderer.aMeshBlock.uNumVertNormals[iMesh] = 0;
			s_renderer.aMeshBlock.uVertexNormalBuffer[iMesh] = 0;
		}

		asset_set_load_state( assetid, load_state == ALS_UNLOAD_RELOAD ? ALS_UNLOADED_RELOAD : ALS_UNLOADED );
		
		ready = 0;
		break;

	default:
		ready = 0;
		break;
	}
	return ready;
}
