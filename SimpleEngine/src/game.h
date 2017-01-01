/*************************************************************************************
  Phyzaks
  Copyright (c) 2012 John Rohrssen
  Email: johnrohrssen@gmail.com
*************************************************************************************/

#ifndef PHYZAKS_GAME_H_INCLUDED
#define PHYZAKS_GAME_H_INCLUDED

#include "util.h"
#include "game_block.h"

#include "math_game.h"

int game_begin_initialize();
int game_initialize();
int game_update();
int game_shutdown();
int game_load_level( int level_assetid );
int game_run( const char* pArgs );

// UI
int game_update_ui( float fDT );
int game_draw_ui( float fDt );

// Objects
int game_create_object(int assetid);
int game_update_objects(float fDt);

int game_object_loaded(int obj);
int game_object_set_transform( int id, Matrix* pMat);
int game_object_get_transform( int id, Matrix* pMat);

int game_object_update_transform( int id, Matrix* pMat);



struct GameObject
{
	enum LoadState
	{
		LS_ASSET_LOAD=0,
		LS_LOADED=1,
		LS_READY=2
	};
};


struct GameObjectInitialData
{
	bool   bApplied;
	Matrix mInitialTransform;
};

struct GameObjectBlock
{
	enum Size { BLOCK_SIZE=1024 };

	int iTail;
	MEMBER( int, iLoadState );
	MEMBER( int, iAssetId);
	MEMBER( uint32, uRenderObject );
	MEMBER( uint32, uPhysicsObject );
	MEMBER( GameObjectInitialData, mInitialData);
};



struct GameLevel
{
	Vector4 vGravity;
};

#define GAMEDATA_MEMBER( type, name, doc ) type name; \/\/ doc 
#define BEGIN_GAMEDATA( name ) struct GameData_##name## \
{
	

#define END_GAMEDATA };


enum GameState
{
	GS_INITIAL,
	GS_BEGIN_INITIALIZING,
	GS_CLIENT_CONNECT,
	GS_CLIENT_CONNECTING,
	GS_CLIENT_CONNECTED,
	GS_CLIENT_FETCHING_ID,
	GS_CLIENT_READY,
	GS_LOADING,
	GS_RUNNING,
	GS_CLOSING
};

struct GameData
{
	GameState gameState;
	GameObjectBlock aObjects;
	int iLevelAssetId;
	Vector4 vGravity;
};

GameData& game_data();
#define s_game game_data()

GameState game_state();
void game_set_state( GameState state );

#endif
