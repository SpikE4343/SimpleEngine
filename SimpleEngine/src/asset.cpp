/*************************************************************************************
  Phyzaks
  Copyright (c) 2012 John Rohrssen
  Email: johnrohrssen@gmail.com
*************************************************************************************/

#include "config.h"
#include "util.h"
#include "system.h"
#include "asset.h"
#include "game_block.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char* asset_block_get_data(AssetBlock* pBlock )
{
	return &pBlock->uData;
}

void print_memory( char* pBuffer )
{
	debug_log( "%x %x %x %x", pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]);
	debug_log( "%x %x %x %x", pBuffer[4], pBuffer[5], pBuffer[6], pBuffer[7]);
	debug_log( "%x %x %x %x", pBuffer[8], pBuffer[9], pBuffer[10], pBuffer[11]);
}

int asset_load_from_buffer( Asset* pAsset, char* pBuffer, int iBufferLen )
{
	//print_memory( pBuffer );
	//print_memory( pBuffer + sizeof(AssetBlockBase) );

	// fixup pointer offsets for asset datablocks in the buffer
	pAsset->pMainBlock = (AssetBlockBase*)pBuffer;
	pAsset->pIndexBlock = (AssetBlockBase*)(pBuffer + sizeof(AssetBlockBase));

	pAsset->u32NextLoadBlock = 0;
	pAsset->u32NumDataBlocks = pAsset->pIndexBlock->u32Size / sizeof( IndexEntryAssetBlock );
	//debug_log( "num data blocks %u, %u, %u", pAsset->u32NumDataBlocks, pAsset->pIndexBlock->u32Size, sizeof( IndexEntryAssetBlock ));

	if( pAsset->u32NumDataBlocks >= ASSET_MAX_DATA_BLOCKS )
		return 0;

	char* pIndexEntryStart = pBuffer + sizeof( AssetBlockBase ) * 2;
	char* pDataStart = pIndexEntryStart + pAsset->pIndexBlock->u32Size;

	for( int i=0; i < pAsset->u32NumDataBlocks; ++i )
	{
		pAsset->pIndexBlocks[i] = (IndexEntryAssetBlock*)(pIndexEntryStart + i * sizeof( IndexEntryAssetBlock ));
		pAsset->pDataBlock[i] = (AssetBlock*)(pDataStart + pAsset->pIndexBlocks[i]->u32Offset);
	}

	return 1;
}

int asset_save_to_system_asset( system_asset* file, Asset* pAsset )
{
	verify( pAsset->u32NumDataBlocks < ASSET_MAX_DATA_BLOCKS);

	// build index block
	uint32 u32Offset = 0;
	for( int i=0; i < pAsset->u32NumDataBlocks; ++i )
	{
		AssetBlock* pBlock = pAsset->pDataBlock[i];
		IndexEntryAssetBlock* pIndex = pAsset->pIndexBlocks[i];
		pIndex->u32FourCC = pBlock->u32FourCC;
		pIndex->u32Offset = u32Offset;
		pIndex->u32Size = pBlock->u32Size;

		u32Offset += pBlock->u32Size + sizeof(AssetBlockBase);
	}

	pAsset->pIndexBlock->u32Size = pAsset->u32NumDataBlocks * sizeof(IndexEntryAssetBlock);

	system_asset_write( file, (char*)pAsset->pMainBlock, sizeof( AssetBlockBase ) );
	system_asset_write( file, (char*)pAsset->pIndexBlock, sizeof( AssetBlockBase ) );

	// write the index to the file
	for( int i=0; i < pAsset->u32NumDataBlocks; ++i )
	{
		system_asset_write( file, (char*)pAsset->pIndexBlocks[i], sizeof(IndexEntryAssetBlock));
	}

	// write actual data to file
	for( int i=0; i < pAsset->u32NumDataBlocks; ++i )
	{
		AssetBlock* pBlock = pAsset->pDataBlock[i];

		system_asset_write( file, (char*)pBlock, sizeof( AssetBlockBase ) );
		system_asset_write( file, asset_block_get_data(pBlock), pBlock->u32Size );
	}
	return 1;
}

int asset_block_type( AssetBlockBase* block, uint32 u32FourCC )
{
	block->u32FourCC = u32FourCC;
	return 1;
}


#define MAX_CLUMP_FILES 10

struct AssetManagerData
{
	enum Size { BLOCK_SIZE=256 };
	bool bLoaderActive;
	int iTail;
	MEMBER( AssetLoadState, iAssetState );
	MEMBER( Asset*,         pAsset );
	MEMBER( int,            iAssetId );
	MEMBER( char*,          pBuffer );
	MEMBER( int,            iBufferLength );
	MEMBER( int,            iCurrentLoadedBlock );

	int iNumClumpFiles;
	const char* aClumpFiles [MAX_CLUMP_FILES];
};



int assetmanager_loader_thread( void* data );

static AssetManagerData s_assetmanager;

Asset* assetmanager_get_data( int assetid )
{
	int size = s_assetmanager.iTail;
	for(int i =0; i < size; ++i)
	{
		if( s_assetmanager.iAssetId[i] == assetid )
		{
			return s_assetmanager.pAsset[i];
		}
	}

	return NULL;
}

int assetmanager_initialize(bool bStartLoaderThread)
{
	s_assetmanager.iTail = 0;
	s_assetmanager.bLoaderActive = true;
	s_assetmanager.iNumClumpFiles = 0;

	if( bStartLoaderThread )
	{
		if( system_create_thread(assetmanager_loader_thread, (void*)&s_assetmanager) == -1 )
			return 0;
	}
	
	return 1;
}

int assetmanager_loader_update()
{
	//PROFILE_FUNC();
	//debug_log( "assetmanager_loader_update()" );
	int size = s_assetmanager.iTail;
	for(int i =0; i < size; ++i)
	{
		switch( s_assetmanager.iAssetState[i] )
		{
		case ALS_NONE:
			break;

		case ALS_WAITING:
			// waiting to load
			{
				debug_log( "assetmanager_loader_update: %d, loading", s_assetmanager.iAssetId[i] );
				s_assetmanager.pAsset[i] = new Asset();
				s_assetmanager.iAssetState[i] = ALS_LOADING;

				s_assetmanager.pBuffer[i] = asset_load_buffer(s_assetmanager.iAssetId[i],s_assetmanager.iBufferLength[i]);

				if( s_assetmanager.pBuffer[i] == NULL )
				{
					debug_log( "assetmanager_loader_update: %d, failed to load asset", s_assetmanager.iAssetId[i] );
					s_assetmanager.iAssetState[i] = ALS_FAILED;
					break;
				}
				else
				{
					debug_log( "assetmanager_loader_update:  %d, file buffer loaded", s_assetmanager.iAssetId[i] );
					s_assetmanager.iAssetState[i] = ALS_BUFFER_LOADED;
				}

				return 1;
			}
			break;

		case ALS_BUFFER_LOADED:
			if( s_assetmanager.pBuffer[i] == NULL ||
				asset_load_from_buffer(s_assetmanager.pAsset[i], s_assetmanager.pBuffer[i], s_assetmanager.iBufferLength[i] ) != 1)
			{
				debug_log( "assetmanager_loader_update: %d, buffer load failed", s_assetmanager.iAssetId[i] );
				s_assetmanager.iAssetState[i] = ALS_FAILED;
				break;
			}

			debug_log( "assetmanager_loader_update: %d, buffer load complete", s_assetmanager.iAssetId[i] );

			s_assetmanager.iAssetState[i] = ALS_LOADED;
			return 1;
			break;

		case ALS_UNLOADING:
		case ALS_UNLOADING_RELOAD:
			// clean up asset data

			debug_log( "assetmanager_loader_update:  %d, unloading", s_assetmanager.iAssetId[i] );
			if( s_assetmanager.pAsset[i] != NULL )
			{
				delete s_assetmanager.pAsset[i];
				s_assetmanager.pAsset[i] = NULL;
			}

			

			if( s_assetmanager.pBuffer[i] != NULL )
			{
				delete[] s_assetmanager.pBuffer[i];
				s_assetmanager.pBuffer[i] = NULL;
			}

			s_assetmanager.iBufferLength[i] = 0;
			s_assetmanager.iAssetState[i] = s_assetmanager.iAssetState[i] == ALS_UNLOADING_RELOAD ? ALS_UNLOADED_RELOAD : ALS_UNLOADED;

			break;

		case ALS_UNLOAD_RELOAD:
			debug_log( "assetmanager_loader_update:  %d, unload reload", s_assetmanager.iAssetId[i] );
			s_assetmanager.iAssetState[i] = ALS_WAITING;
			break;
		}

	}
	return 1;
}

int assetmanager_shutdown()
{
	s_assetmanager.bLoaderActive = false;
	return 0;
}

AssetLoadState asset_load_state( int assetid )
{
	PROFILE_FUNC();
	for(int i =0; i < s_assetmanager.iTail; ++i)
	{
		if( s_assetmanager.iAssetId[i] == assetid)
			return s_assetmanager.iAssetState[i];
	}
	return ALS_NONE;
}

int asset_load_state_current_block( int assetid )
{
	for(int i =0; i < s_assetmanager.iTail; ++i)
	{
		if( s_assetmanager.iAssetId[i] == assetid)
			return s_assetmanager.iCurrentLoadedBlock[i];
	}
}

void asset_load_state_set_current_block( int assetid, int block )
{
	for(int i =0; i < s_assetmanager.iTail; ++i)
	{
		if( s_assetmanager.iAssetId[i] == assetid)
		{
			s_assetmanager.iCurrentLoadedBlock[i] = block;
			return;
		}
	}
}


void asset_set_load_state( int assetid, AssetLoadState state )
{
	for(int i =0; i < s_assetmanager.iTail; ++i)
	{
		if( s_assetmanager.iAssetId[i] == assetid)
		{
			s_assetmanager.iAssetState[i] = state;
			return;
		}
	}
}

char* asset_load_buffer( int assetid, int& size )
{
	char* buffer = NULL;
	size=0;

	for( int i=0; i < s_assetmanager.iNumClumpFiles; ++i )
	{
		buffer = asset_load_buffer_from_clump( s_assetmanager.aClumpFiles[i], assetid, size );
		if( buffer != NULL )
			return buffer;
	}

	return buffer;
}

char* asset_load_buffer_from_clump( const char* clump, int assetid, int& size )
{
	system_asset* pClump = system_asset_open( clump);
	if( pClump == NULL )
	{
		system_log( "failed to load clump file: %s", clump );
		return 0;
	}

	AssetBlockBase main, index;

	// Read Main Block
	system_asset_read( pClump, (char*)&main, sizeof(AssetBlockBase) );
	if( main.u32FourCC != AT_ASSET_CLUMP )
	{
		system_asset_close(pClump);
		return NULL;
	}

	// Read Index Block
	system_asset_read( pClump, (char*)&index, sizeof(AssetBlockBase) );
	if( index.u32FourCC != AT_ASSET_INDEX )
	{
		system_asset_close(pClump);
		return NULL;
	}

	// Search Index for asset id;
	int iNumIndexEntries = index.u32Size / sizeof( IndexEntryAssetBlock );
	debug_log( "iNumIndexEntries: %d", iNumIndexEntries);

	int iDataStartPos = system_asset_getpos(pClump) + index.u32Size;
	debug_log( "iDataStartPos: %d", iDataStartPos);

	IndexEntryAssetBlock entry;
	bool bFound = false;
	for( int i=0; i < iNumIndexEntries; ++i )
	{
		system_asset_read( pClump, (char*)&entry, sizeof(IndexEntryAssetBlock) );
		if( entry.u32AssetId == assetid )
		{
			bFound = true;
			break;
		}
	}

	if( !bFound )
	{
		system_asset_close(pClump);
		return NULL;
	}

	debug_log( "entry.u32Offset: %u", entry.u32Offset);
	debug_log( "entry.u32Size: %u", entry.u32Size);
	system_asset_setpos( pClump, iDataStartPos + entry.u32Offset );

	size = entry.u32Size;
	char* file = new char[size];
	char* pCurrent = file;

	int bytesRead = 0;
	while( (bytesRead = system_asset_read(pClump, pCurrent, size)) < size )
	{
		pCurrent += bytesRead;
	}

	system_asset_close(pClump);

	return file;
}

int assetmanager_load( int assetid )
{
	PROFILE_FUNC();
	if( s_assetmanager.iTail >= MAX_LOADED_ASSETS)
		return -1;

	for(int i =0; i < s_assetmanager.iTail; ++i)
	{
		switch( s_assetmanager.iAssetState[i] )
		{
		case ALS_NONE:
			break;

		case ALS_LOADING:
		case ALS_BUFFER_LOADED:
		case ALS_WAITING:
		case ALS_LOADED:

		case ALS_UNLOADING:
		case ALS_UNLOADING_RELOAD:
		case ALS_UNLOAD:
		case ALS_UNLOAD_RELOAD:
		case ALS_UNLOADED:
		case ALS_UNLOADED_RELOAD:
			if( s_assetmanager.iAssetId[i] == assetid )
			{
				return assetid;
			}
			break;
		}
	}

	// couldn't find an existing asset
	int nextid = s_assetmanager.iTail++;
	s_assetmanager.iAssetId [nextid] = assetid;
	s_assetmanager.iCurrentLoadedBlock[nextid] = 0;
	s_assetmanager.iAssetState[nextid] = ALS_WAITING;

    return assetid;
}

AssetLoadState assetmanager_unload( int assetid )
{
	PROFILE_FUNC();
	for(int i =0; i < s_assetmanager.iTail; ++i)
	{
		if( s_assetmanager.iAssetId[i] == assetid)
		{
			if( s_assetmanager.iAssetState[i] == ALS_UNLOADED )
				return ALS_UNLOADED;

			if( s_assetmanager.iAssetState[i] == ALS_UNLOAD )
				return ALS_UNLOAD_RELOAD;

			if( s_assetmanager.iAssetState[i] == ALS_UNLOADING )
				return ALS_UNLOADING;

			s_assetmanager.iAssetState[i] = ALS_UNLOADING;
			return ALS_UNLOAD;
		}
	}

	return ALS_NONE;
}

AssetLoadState assetmanager_reload( int assetid )
{
	PROFILE_FUNC();
	for(int i =0; i < s_assetmanager.iTail; ++i)
	{
		if( s_assetmanager.iAssetId[i] == assetid)
		{
			if( s_assetmanager.iAssetState[i] == ALS_UNLOADED_RELOAD )
				return ALS_UNLOADED_RELOAD;

			if( s_assetmanager.iAssetState[i] == ALS_UNLOAD_RELOAD )
				return ALS_UNLOAD_RELOAD;

			if( s_assetmanager.iAssetState[i] == ALS_UNLOADING_RELOAD )
				return ALS_UNLOADING_RELOAD;

			s_assetmanager.iAssetState[i] = ALS_UNLOADING_RELOAD;
			return ALS_UNLOAD_RELOAD;
		}
	}

	return ALS_NONE;
}

int assetmanager_isloaded( int assetid )
{
	PROFILE_FUNC();
	for(int i =0; i < s_assetmanager.iTail; ++i)
	{
		if( s_assetmanager.iAssetId[i] == assetid)
		{
			return s_assetmanager.iAssetState[i] == ALS_LOADED;
		}
	}

	return false;
}

int assetmanager_isloading( int assetid )
{
	PROFILE_FUNC();
	for(int i =0; i < s_assetmanager.iTail; ++i)
	{
		if( s_assetmanager.iAssetId[i] == assetid)
		{
			return s_assetmanager.iAssetState[assetid] == ALS_LOADING ||
				   s_assetmanager.iAssetState[assetid] == ALS_BUFFER_LOADED ||
				   s_assetmanager.iAssetState[assetid] == ALS_WAITING;

		}
	}

	return false;
}

int assetmanager_loader_thread( void* data )
{
	while( s_assetmanager.bLoaderActive )
	{
		assetmanager_loader_update();

		system_sleep( 100 );
	}

	return 1;
}

int assetmanager_add_clump( const char* clumpfile )
{
	s_assetmanager.aClumpFiles[s_assetmanager.iNumClumpFiles++] = strdup(clumpfile);
	return s_assetmanager.iNumClumpFiles;
}

int assetmanager_remove_clump( const char* clumpfile )
{
	for( int i=0; i < s_assetmanager.iNumClumpFiles; ++i )
	{
		if( strcmp( s_assetmanager.aClumpFiles[i],clumpfile) == 0 )
		{
			delete[] s_assetmanager.aClumpFiles[i];
			return 1;
		}
	}
	
	return 0;
}


#if !SYSTEM_ANDROID
int assetmanager_build_clump_file( const char* clumpfilename, const char* indexfilename)
{
	system_asset* clump = system_asset_open(clumpfilename, SOM_WRITE);
	if( clump == NULL )
	{
		system_log( "assetmanager_build_clump_file failed to open clump file: %s", clumpfilename );
		return 0;
	}

	system_asset* index = system_asset_open(indexfilename, SOM_READ_TEXT);
	if( index == NULL )
	{
		system_log( "assetmanager_build_clump_file failed to open index file: %s", index );
		return 0;
	}

	AssetBlockBase assetClump;
	assetClump.u32FourCC = AT_ASSET_CLUMP;
	assetClump.u32Version = 0;

	AssetBlockBase assetClumpIndex;
	assetClumpIndex.u32FourCC = AT_ASSET_INDEX;
	assetClumpIndex.u32Version = 0;

	int assetIds[ASSET_MAX_DATA_BLOCKS];
	char* assetFileNames[ASSET_MAX_DATA_BLOCKS];
	char line[4096];

	int numAssets = 0;
	int lineLen = 0;
	while( system_asset_gets(index, line, 1024) )
	{
		if( numAssets >= ASSET_MAX_DATA_BLOCKS)
			break;

		int id = atoi(util_strtok(line,":"));
		char* file = strdup(util_strtok(NULL,":"));

		lineLen = strlen(file);
		int c = lineLen -1;
		while( c > 0 && isalnum(file[c]) == 0  )
		{
			file[c]=0;
			 --c;
		}


		debug_log( "%d:%s", id, file);

		assetIds[numAssets] = id;
		assetFileNames[numAssets] = file;
		++numAssets;
	}

	system_asset_close(index);

	system_asset_write(clump, (char*)&assetClump, sizeof( AssetBlockBase));

	assetClumpIndex.u32Size = numAssets * sizeof( IndexEntryAssetBlock );
	system_asset_write(clump, (char*)&assetClumpIndex, sizeof( AssetBlockBase));

	int currentIndexPos = system_asset_getpos(clump);

	// reserve space for the asset index
	int writeCount = assetClumpIndex.u32Size / 4096;
	for( int i=0; i < writeCount; ++i)
	{
		system_asset_write( clump, line, 4096 );
	}

	writeCount = assetClumpIndex.u32Size % 4096;
	if( writeCount > 0 )
	{
		system_asset_write( clump, line, writeCount );
	}

	int assetStartPos = system_asset_getpos(clump);
	int currentAssetPos = assetStartPos;

	IndexEntryAssetBlock tempIndex;
	for( int i=0; i < numAssets; ++i)
	{
		int id = assetIds[i];
		char* file = assetFileNames[i];

		debug_log( "Clumping asset %d:%s", id, file);

		system_asset* asset_file = system_asset_open( file, SOM_READ);
		if( asset_file == NULL )
			continue;

		AssetBlockBase assetMainBlock;
		system_asset_read(asset_file, (char*)&assetMainBlock, sizeof(AssetBlockBase)  );

		// update index entry based on asset
		tempIndex.u32FourCC = assetMainBlock.u32FourCC;
		tempIndex.u32AssetId = id;
		tempIndex.u32Size = assetMainBlock.u32Size + sizeof( AssetBlockBase );
		tempIndex.u32Offset = currentAssetPos - assetStartPos;

		// write out updated index entry
		system_asset_setpos(clump, currentIndexPos);
		system_asset_write(clump, (char*)&tempIndex, sizeof(IndexEntryAssetBlock));
		currentIndexPos = system_asset_getpos(clump);

		// jump back to the next asset block and write it out
		system_asset_setpos(clump, currentAssetPos);

		system_asset_write(clump, (char*)&assetMainBlock, sizeof(AssetBlockBase));

		int bytesRead=0;
		while( bytesRead = system_asset_read(asset_file, line, 4096))
		{
			system_asset_write(clump, line, bytesRead);
		}

		currentAssetPos = system_asset_getpos(clump);

		system_asset_close(asset_file);

		delete[] file;
	}

	assetClump.u32Size = currentAssetPos - sizeof( AssetBlockBase );
	system_asset_setpos(clump, 0);
	system_asset_write(clump, (char*)&assetClump, sizeof( AssetBlockBase));

	system_asset_close(clump);
	return 1;
}
#endif
