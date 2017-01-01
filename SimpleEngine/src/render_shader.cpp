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

int render_create_shader(int assetid)
{
	int index = -1;
	if( assetid > 0 )
	{
		bool bFound = false;
		for( int i=1; i < s_renderer.aShaderBlock.iTail; ++i )
		{
			if( s_renderer.aShaderBlock.aShaders[i].iAssetId == assetid )
			{
				bFound = true;
				index = i;
				break;
			}
		}

		assetmanager_load( assetid );
	}

	// to many objects in the block
	if( s_renderer.aShaderBlock.iTail < 0 || 
		s_renderer.aShaderBlock.iTail - 1 >= RenderObjectBlock::BLOCK_SIZE )
		return -1;

	if( index == -1 )
	{
		index = s_renderer.aShaderBlock.iTail++;
		Shader& shader = s_renderer.aShaderBlock.aShaders[index];
		shader.iAssetId = assetid;
	}
	return index;
}