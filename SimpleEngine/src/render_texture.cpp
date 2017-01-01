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

int render_create_texture(int assetid)
{
	int index = -1;
	if( assetid > 0 )
	{
		bool bFound = false;
		for( int i=1; i < s_renderer.aTextureBlock.iTail; ++i )
		{
			if( s_renderer.aTextureBlock.iAssetId[i] == assetid )
			{
				bFound = true;
				index = i;
				break;
			}
		}
		
		assetmanager_load( assetid );
	}

	if( index == -1 )
	{
		// to many objects in the block
		if( s_renderer.aTextureBlock.iTail - 1 >= RenderObjectBlock::BLOCK_SIZE )
			return -1;

		index = s_renderer.aTextureBlock.iTail++;

		s_renderer.aTextureBlock.uHeight[index]   = 0;
		s_renderer.aTextureBlock.uWidth[index]    = 0;
		s_renderer.aTextureBlock.uTexture[index]  = 0;
		s_renderer.aTextureBlock.iAssetId[index]  = assetid;
	}

	return index;
}

void render_get_texture_info( int iTextureId, int& iWidth, int& iHeight )
{
	if( iTextureId < 0 || iTextureId >= s_renderer.aTextureBlock.iTail )
		return;

	iWidth  = s_renderer.aTextureBlock.uWidth[iTextureId];
	iHeight = s_renderer.aTextureBlock.uHeight[iTextureId];
}