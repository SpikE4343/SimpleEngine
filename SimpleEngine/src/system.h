/*************************************************************************************
  Phyzaks
  Copyright (c) 2012 John Rohrssen
  Email: johnrohrssen@gmail.com
*************************************************************************************/

#ifndef PHYZAKS_SYSTEM_H_INCLUDED
#define PHYZAKS_SYSTEM_H_INCLUDED

#include "config.h"
#include "util.h"

#define MAX_NETWORK_SOCKETS 8

struct Vector4;

typedef void* SystemNativeDisplayType;
typedef void* SystemNativeWindowType;

int   system_mouse_delta_x();
int   system_mouse_delta_y();

int   system_mouse_x();
int   system_mouse_y();

int   system_initialize(void* pNativeData);
int   system_update();
int   system_shutdown();
bool  system_isKeyDown(int iKey);
bool  system_mouse_button_down(int iButton);
int   system_createWindow ( int width, int height, const char *title );

int system_set_mouse_pos( int x, int y );

float system_get_time_secs();
float system_get_frame_time_secs();

void* system_display();
int   system_display_created( int iFormat );
void* system_window();

void system_exit(int error=0);
void system_sleep( int ms );

void system_get_accelerometer( Vector4& vec);
const Vector4& system_get_accelerometer();

enum NetworkConnectionProtocol
{
	NCP_TCP,
	NCP_UDP
};

// networking 

int system_network_initialize();
int system_network_shutdown();

int system_network_create_socket(NetworkConnectionProtocol conType );
int system_network_destroy_socket( int socket );

int system_network_connect(int socket, const char* address, int port);
int system_network_bind(int socket, int port);

int system_network_send( int socket, const char* buffer, int len );
int system_network_recv( int socket, char* buffer, int len );

int system_network_peek( int socket, char* buffer, int len );

int system_network_listen( int socket, int max_pending_connections );
int system_network_accept( int socket );

int system_network_can_read( int socket );
int system_network_can_write( int socket );
int system_network_status( int socket, bool& can_write, bool& can_read, bool& has_error );


#if SYSTEM_ANDROID

#include <android_native_app_glue.h>
#include <android/log.h>

#define system_log(...) ((void)__android_log_print(ANDROID_LOG_ERROR, PHYZAKS_APP_NAME, __VA_ARGS__))
#define system_crash(...) ((void)system_log(__VA_ARGS__)); system_sleep(1000); system_exit(1);

#include <android/asset_manager.h>
typedef AAsset system_asset;
#else
#define system_log(...) printf(__VA_ARGS__); printf("\n");

#define system_crash(...) system_log(__VA_ARGS__); system_sleep(1000); system_exit(1);

#include <stdio.h>
typedef FILE system_asset;
#endif

#if DEBUG
#define debug_log(...) system_log(__VA_ARGS__);
#else
#define debug_log(...)
#endif

enum SystemAssetOpenMode
{
	SOM_READ=0,
	SOM_READ_TEXT,
	SOM_READ_TEXT_APPEND,

	SOM_READ_WRITE,
	SOM_READ_WRITE_APPEND,

	SOM_READ_WRITE_TEXT,
	SOM_READ_WRITE_TEXT_APPEND,

	SOM_WRITE,
	SOM_WRITE_APPEND,

	SOM_WRITE_TEXT,
	SOM_WRITE_TEXT_APPEND

};

char*         system_asset_load_file       ( const char* sFile, int& size);
system_asset* system_asset_open            ( const char* sFile, SystemAssetOpenMode Mode=SOM_READ );
int           system_asset_read            ( system_asset* asset, char* buffer, int max_length);
int 		  system_asset_write           ( system_asset* asset, char* buffer, int length);
int           system_asset_length          ( system_asset* asset );
int           system_asset_bytes_remaining ( system_asset* asset );
void          system_asset_close           ( system_asset* asset );
const void*   system_asset_buffer          ( system_asset* asset );
int           system_asset_getpos		   ( system_asset* asset );
int           system_asset_setpos		   ( system_asset* asset, int pos );
int           system_asset_gets  		   ( system_asset* asset, char* str, int max );


typedef int (*system_thread_call) (void* userdata);

int system_create_thread(system_thread_call, void* data=NULL);
int system_current_thread();

#if DEBUG
	#if SYSTEM_WIN32
	#include <Windows.h>
		#define verify(e) if( (e) == false ) { system_log( "assertion %s failed: %s:%d", #e, __FILE__, __LINE__); system_sleep(1000); DebugBreak(); }
	#else
		#define verify(e) if( (e) == false ) { system_log( "assertion %s failed: %s:%d", #e, __FILE__, __LINE__); system_sleep(1000); system_exit(1); }
	#endif
#else
	#define verify(e) if( (e) == false ) { system_log( "assertion %s failed: %s:%d", #e, __FILE__, __LINE__); system_sleep(1000); system_exit(1); }
#endif


struct Input
{
	enum Keys
	{
		K_ESCAPE = 0x1B,
		K_SPACE = 0x20,
		K_W = 'W',
		K_A = 'A',
		K_S = 'S',
		K_D = 'D',
		K_Q = 'Q',
		K_E = 'E',
		K_X = 'X',
	};

	enum MouseButtons
	{
		LEFT_BUTTON = 0,
		RIGHT_BUTTON = 1
	};
};


#endif
