/*************************************************************************************
Phyzaks
Copyright (c) 2012 John Rohrssen
Email: johnrohrssen@gmail.com
*************************************************************************************/

#ifndef ASSET_H_
#define ASSET_H_

#include "util.h"

#define FOURCC( a, b, c, d) ((d << 24) | (c << 16) | (b << 8) | (a << 0))

#define MAX_LOADED_ASSETS 128

enum AssetLoadState
{
  ALS_NONE= 0,
  ALS_WAITING,
  ALS_BUFFER_LOADED,
  ALS_LOADING,
  ALS_LOADED,
  ALS_READY,
  ALS_UNLOADING,
  ALS_UNLOADING_RELOAD,
  ALS_UNLOAD,
  ALS_UNLOAD_RELOAD,
  ALS_UNLOADED,
  ALS_UNLOADED_RELOAD,
  ALS_FAILED,

};

enum AssetTypes
{
  AT_ASSET						= FOURCC('a','s','s','t'),
  AT_ASSET_INDEX					= FOURCC('i','n','d','x'),

  AT_ASSET_CLUMP					= FOURCC('c','l','m','p'),

  AT_GAME_MODEL					= FOURCC('g','m','d','l'),
  AT_GAME_MODEL_INFO				= FOURCC('g','m','i',' '),
  AT_GAME_MODEL_RENDER			= FOURCC('g','m','r',' '),
  AT_GAME_MODEL_PHYSICS    		= FOURCC('g','m','p',' '),

  AT_GAME_LEVEL					= FOURCC('g','l','v','l'),
  AT_GAME_LEVEL_INFO				= FOURCC('g','l','i',' '),
  AT_GAME_LEVEL_OBJECT			= FOURCC('g','l','o',' '),

  AT_RENDER_OBJECT				= FOURCC('r','o','b','j'),
  AT_RENDER_OBJECT_INFO			= FOURCC('r','o','i',' '),
  AT_RENDER_OBJECT_MESH  		    = FOURCC('r','o','m',' '),

  AT_RENDER_MESH					= FOURCC('r','m','e','s'),
  AT_RENDER_MESH_VERTEX			= FOURCC('r','v','x',' '),
  AT_RENDER_MESH_VERTEX_NORMAL	= FOURCC('r','v','n',' '),
  AT_RENDER_MESH_INDEX			= FOURCC('r','i','d',' '),
  AT_RENDER_MESH_UV				= FOURCC('r','u','v',' '),

  AT_RENDER_EFFECT				= FOURCC('r','e','f','t'),
  AT_RENDER_VERTEX_SHADER			= FOURCC('r','v','s',' '),
  AT_RENDER_FRAGMENT_SHADER		= FOURCC('r','f','s',' '),
  AT_RENDER_SHADER_PARAM			= FOURCC('r','s','p',' '),
  AT_RENDER_SHADER_ATTRIBUTE		= FOURCC('r','s','a',' '),
  AT_RENDER_SHADER_STATE			= FOURCC('r','s','s',' '),
  AT_RENDER_SHADER_TEXTURE_SAMPLE = FOURCC('r','s','t',' '),

  AT_RENDER_FONT					= FOURCC('r','f','n','t'),
  AT_RENDER_FONT_INFO				= FOURCC('r','f','i',' '),

  AT_TEXTURE						= FOURCC('t','x','t','r'),
  AT_TEXTURE_INFO					= FOURCC('t','x','i',' '),
  AT_TEXTURE_DATA					= FOURCC('t','x','d',' '),

  AT_PHYSICS						= FOURCC('p','o','b','j'),
  AT_PHYSICS_COLLISION			= FOURCC('p','o','c',' '),
  AT_PHYSICS_DYNAMICS				= FOURCC('p','o','d',' '),
};

PACKED(
struct AssetBlockBase
{
  uint32 u32FourCC;
  uint32 u32Version;
  uint32 u32Size;
});

PACKED(
struct IndexEntryAssetBlock : AssetBlockBase
{
  uint32 u32Offset;
  uint32 u32AssetId;
});

#define ASSET_MAX_DATA_BLOCKS 64

PACKED(
struct AssetBlock : public AssetBlockBase
{
  // asset block that has been loaded into memory
  char uData;
});

PACKED(
struct RenderObjectInfoAssetBlock : public AssetBlockBase
{
  // asset block that has been loaded into memory
  float bounds_min[3];
  float bounds_max[3];
  char  shader_name[32];
});

PACKED(
struct GameLevelInfoAssetBlock : public AssetBlockBase
{
  // asset block that has been loaded into memory
  float gravity[3];
});

PACKED(
struct GameLevelObjectAssetBlock : public AssetBlockBase
{
  // asset block that has been loaded into memory
  int iAssetId;
  float world_matrix[16];
});

PACKED(
struct TextureInfoAssetBlock : public AssetBlockBase
{
  // asset block that has been loaded into memory
  int iWidth;
  int iHeight;
  int iBitPerPixel;
});

PACKED(
struct FontInfoAssetBlock : public AssetBlockBase
{
  // asset block that has been loaded into memory
  int iWidth;
  int iHeight;
  int iTextureAssetId;
  int iEffectAssetId;
});

PACKED(
struct Asset
{
  AssetBlockBase* pMainBlock;
  AssetBlockBase* pIndexBlock;
  IndexEntryAssetBlock* pIndexBlocks[ASSET_MAX_DATA_BLOCKS];
  AssetBlock*     pDataBlock[ASSET_MAX_DATA_BLOCKS];
  uint32 u32NumDataBlocks;
  uint32 u32NextLoadBlock;

});

char* asset_block_get_data(AssetBlock* pBlock );
int asset_load_from_buffer( Asset* pAsset, char* pBuffer, int iBufferLen );
int asset_save_to_system_asset( system_asset* file, AssetBlockBase* pAsset );

int asset_add_block( Asset* pAsset, AssetBlockBase* pBlock);
AssetBlockBase* asset_remove_block( Asset* pAsset, int iIndex);

AssetLoadState asset_load_state( int assetid );
int asset_load_state_current_block( int assetid );
void asset_load_state_set_current_block( int assetid, int block );
void asset_set_load_state( int assetid, AssetLoadState state );

char* asset_load_buffer( int assetid, int& size );
char* asset_load_buffer_from_clump( const char* clump, int assetid, int& size );

int assetmanager_initialize(bool bStartLoaderThread=true);
int assetmanager_loader_update();
int assetmanager_shutdown();
int assetmanager_load( int assetid );
AssetLoadState assetmanager_unload( int assetid );
AssetLoadState assetmanager_reload( int assetid );
int assetmanager_isloaded( int assetid );
int assetmanager_isloading( int assetid );
Asset* assetmanager_get_data( int assetid );

int assetmanager_add_clump( const char* clumpfile );
int assetmanager_remove_clump( const char* clumpfile );

int assetmanager_build_clump_file( const char* clumpfilename, const char* indexfilename);

#endif /* ASSET_H_ */
