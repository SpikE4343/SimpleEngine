/*************************************************************************************
  Phyzaks
  Copyright (c) 2012 John Rohrssen
  Email: johnrohrssen@gmail.com
*************************************************************************************/

#include "config.h"

#if SYSTEM_ANDROID 

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "system.h"
#include "render.h"
#include "game.h"

#include <jni.h>
#include <errno.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define NUM_KEYS 256
#define NUM_MOUSE_BUTTONS 4

int32_t system_handle_input(struct android_app* app, AInputEvent* event);
void system_handle_command(struct android_app* app, int32_t cmd);

struct System
{
	void* displayType;

	struct android_app* app;

	AAssetManager* pAssetManager;
	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	float fFrameTimeSecs;
	float fStartSysTimeSecs;
	float fLastSysTimeSecs;
	float fClockFreq;

	float fMouseX;
	float fMouseY;
	float fLastMouseX;
	float fLastMouseY;

	bool bShutdown;

	Vector4 vAccelerometer;
};

static System s_system;

int system_initialize(void* pNativeData)
{
	s_system.app = (android_app*) pNativeData;
	s_system.fClockFreq = 1000000.0f;
	s_system.fStartSysTimeSecs = 0;

	s_system.fStartSysTimeSecs = s_system.fLastSysTimeSecs =
			system_get_time_secs();
	s_system.fFrameTimeSecs = 1;
	s_system.bShutdown = false;

	// Make sure glue isn't stripped.
	app_dummy();

	s_system.app->userData = &s_system;
	s_system.app->onAppCmd = system_handle_command;
	s_system.app->onInputEvent = system_handle_input;

	s_system.fLastMouseX = s_system.fMouseX = 10000.0f;
	s_system.fLastMouseY = s_system.fMouseY = 10000.0f;

	// Prepare to monitor accelerometer
	s_system.sensorManager       = ASensorManager_getInstance();
	s_system.accelerometerSensor = ASensorManager_getDefaultSensor(s_system.sensorManager, ASENSOR_TYPE_ACCELEROMETER);


	s_system.sensorEventQueue    = ASensorManager_createEventQueue(s_system.sensorManager, s_system.app->looper, LOOPER_ID_USER, NULL,
			NULL);

	s_system.pAssetManager = s_system.app->activity->assetManager;

	verify( s_system.pAssetManager != NULL);

	return 1;
}

int system_update()
{
	PROFILE_FUNC();
	float fNow = system_get_time_secs();
	s_system.fFrameTimeSecs = (fNow - s_system.fLastSysTimeSecs);
	s_system.fLastSysTimeSecs = fNow;

	int ident = -1;
	int events = 0;
	struct android_poll_source* source = NULL;

	s_system.fLastMouseX = s_system.fMouseX;
	s_system.fLastMouseY = s_system.fMouseY;

	while ((ident = ALooper_pollAll(0, NULL, &events, (void**) &source)) >= 0)
	{
		if (source != NULL)
		{
			source->process(s_system.app, source);
		}

		// If a sensor has data, process it now.
		if (ident == LOOPER_ID_USER)
		{
			if (s_system.accelerometerSensor != NULL)
			{
				ASensorEvent event;
				while (ASensorEventQueue_getEvents(s_system.sensorEventQueue, &event, 1) > 0)
				{
					//system_log("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
					s_system.vAccelerometer.y = -event.acceleration.x;
					s_system.vAccelerometer.x = -event.acceleration.y;
					s_system.vAccelerometer.z = -event.acceleration.z;
				}
			}
		}

		if( s_system.bShutdown )
			return 0;

		// Check if we are exiting.
		if (s_system.app->destroyRequested != 0)
		{
			return 0;
		}
	}

	return 1;
}

int system_shutdown()
{
	return 1;
}

/**
 * Process the next input event.
 */
int32_t system_handle_input(struct android_app* app, AInputEvent* event)
{
	verify( &s_system == app->userData);

	s_system.fLastMouseX = s_system.fMouseX;
	s_system.fLastMouseY = s_system.fMouseY;

	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
	{
		//if( AMotionEvent_getAction(event) ==  AMOTION_EVENT_ACTION_DOWN)
		{

			s_system.fMouseX = AMotionEvent_getX(event, 0);
			s_system.fMouseY = AMotionEvent_getY(event, 0);
		}

		if( AMotionEvent_getAction(event) ==  AMOTION_EVENT_ACTION_DOWN )
		{
			s_system.fLastMouseX = s_system.fMouseX;
			s_system.fLastMouseY = s_system.fMouseY;
		}

		return 1;
	}
	return 0;
}

void system_handle_command(struct android_app* app, int32_t cmd)
{
	verify( &s_system == app->userData);

	struct engine* engine = (struct engine*) app->userData;
	switch (cmd)
	{
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		//engine->app->savedState = malloc(sizeof(struct saved_state));
		//*((struct saved_state*)engine->app->savedState) = engine->state;
		//engine->app->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (s_system.app->window != NULL)
		{
			game_begin_initialize();
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		s_system.bShutdown = true;
		break;

	case APP_CMD_GAINED_FOCUS:
		// When our app gains focus, we start monitoring the accelerometer.
		if (s_system.accelerometerSensor != NULL)
		{
			ASensorEventQueue_enableSensor(s_system.sensorEventQueue, s_system.accelerometerSensor);

			// We'd like to get 60 events per second (in us).
			ASensorEventQueue_setEventRate(s_system.sensorEventQueue, s_system.accelerometerSensor, (1000L / 60) * 1000);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		if (s_system.accelerometerSensor != NULL)
		{
			ASensorEventQueue_disableSensor(s_system.sensorEventQueue,
					s_system.accelerometerSensor);
		}

		break;
	}
}

int system_createWindow(int width, int height, const char *title)
{
	return 1;
}

int system_display_created(int iFormat)
{
	int iwindow = ANativeWindow_getFormat(s_system.app->window);
	system_log( "window format %d", iwindow);
	system_log( "setting system window format %d", iFormat);
	ANativeWindow_setBuffersGeometry(s_system.app->window, 0, 0, iFormat);
	return 1;
}

void* system_display()
{
	return &s_system.displayType;
}

void* system_window()
{
	return &s_system.app->window;
}

// input management

bool system_isKeyDown(int iKey)
{
	return false;
}

bool system_mouse_button_down(int iButton)
{
	return false;
}

int system_mouse_delta_x()
{
	return s_system.fMouseX - s_system.fLastMouseX;
}

int system_mouse_delta_y()
{
	return s_system.fMouseY - s_system.fLastMouseY;
}

int system_mouse_x()
{
	return s_system.fMouseX;
}

int system_mouse_y()
{
	return s_system.fMouseY;
}


float system_get_frame_time_secs()
{
	return s_system.fFrameTimeSecs;
}

float system_get_time_secs()
{
	//timespec spec;
	//clock_gettime(CLOCK_MONOTONIC, &spec );

	timeval time;
	gettimeofday(&time, NULL);

	return (float) (((double) (time.tv_sec * 1000000.0 + time.tv_usec) / (double) s_system.fClockFreq) - s_system.fStartSysTimeSecs);
}

void system_exit(int error)
{
	exit(error);
}

void system_sleep( int ms )
{
	timespec t = { ms/1000, (ms%1000)*1000000 };
	nanosleep(&t, 0);
}

// Asset manager

void system_asset_dump_dir(const char* pDir)
{
	AAssetDir* dir = AAssetManager_openDir(s_system.pAssetManager, pDir);

	if( dir == NULL )
		return;

	const char* pFile = NULL;

	while( (pFile = AAssetDir_getNextFileName(dir)) )
	{
		system_log("asset file: %s", pFile );
		system_asset_dump_dir(pFile);
	}

	AAssetDir_close(dir);
}

system_asset* system_asset_open(const char* sFile, SystemAssetOpenMode iMode)
{
	verify( s_system.pAssetManager != NULL);
	verify( sFile != NULL);


	//
	system_asset_dump_dir("");

	return AAssetManager_open(s_system.pAssetManager, sFile, AASSET_MODE_UNKNOWN );
}

int system_asset_gets( system_asset* asset, char* str, int max )
{
	return 0;
}

int system_asset_read(system_asset* asset, char* buffer, int max_length)
{
	return AAsset_read(asset, (void*) buffer, max_length);
}

int system_asset_write(system_asset* asset, char* buffer, int length)
{
	return 0;
}

int system_asset_length(system_asset* asset)
{
	return (int) AAsset_getLength(asset);
}

int system_asset_bytes_remaining( system_asset* asset )
{
	return (uint32) AAsset_getRemainingLength(asset);
}

void system_asset_close(system_asset* asset)
{
	AAsset_close(asset);
}

const void* system_asset_buffer(system_asset* asset)
{
	return AAsset_getBuffer(asset);
}

int system_asset_getpos( system_asset* asset )
{
	return system_asset_length(asset) - system_asset_bytes_remaining(asset);
}

int system_asset_setpos ( system_asset* asset, int pos )
{
	int p = pos - system_asset_getpos(asset);
	return AAsset_seek(asset, p, 1 );
}

char* system_asset_load_file( const char* sFile, int& size )
{
	system_log( "loading asset file: %s", sFile );
	system_asset* pAsset = system_asset_open( sFile);
	if( pAsset == NULL )
	{
		system_log( "failed to load asset file: %s", sFile );

		return 0;
	}

	size = system_asset_length(pAsset);
	char* file = new char[size+1];
	file[size] = '\0';
	char* pCurrent = file;
	char line[1024];

	int bytesRead = 0;
	while( (bytesRead = system_asset_read(pAsset, pCurrent, 1024)) )
	{
		pCurrent += bytesRead;
	}

	system_asset_close(pAsset);

	return file;
}

void system_get_accelerometer( Vector4& vec)
{
	vec = s_system.vAccelerometer;
}


const Vector4& system_get_accelerometer()
{
	return s_system.vAccelerometer;
}

struct system_thread_startdata
{
	system_thread_call func;
	void* data;
};

void* system_thread_starter(void* start_data)
{
	system_thread_startdata* data = (system_thread_startdata*)start_data;
	system_thread_call func = data->func;
	int ret = (func)(data->data);

	delete data;

	return (void*)ret;
}

int system_create_thread(system_thread_call func, void* data)
{
	system_thread_startdata* start_data = new system_thread_startdata;
	start_data->func = func;
	start_data->data = data;
	int iThreadId=-1;

	pthread_t thread;
	pthread_create(&thread,NULL, system_thread_starter, (void*)start_data);

	iThreadId = (int)thread;
	return iThreadId;
}

int system_current_thread()
{
	return (int)pthread_self();
}

int system_network_initialize()
{
	return 1;
}

int system_network_create_socket(NetworkConnectionProtocol conType )
{
	int socket_proto = IPPROTO_TCP;
	switch( conType )
	{
		case NCP_TCP:
			socket_proto = IPPROTO_TCP;
			break;

		case NCP_UDP:
			socket_proto = IPPROTO_UDP;
			break;
	}

	return socket(PF_INET, SOCK_STREAM,socket_proto );
}

int system_network_destroy_socket( int socket )
{
	return close(socket);
}

int system_network_connect(int socket, const char* address, int port)
{
	struct sockaddr_in remoteAddr;
	memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.sin_family      = AF_INET;
    remoteAddr.sin_addr.s_addr = inet_addr(address);
    remoteAddr.sin_port        = htons(port);

    return connect(socket, (struct sockaddr *) &remoteAddr, sizeof(remoteAddr));
}

int system_network_bind(int socket, int port)
{
	struct sockaddr_in localAddr;

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(port);

    return bind(socket, (struct sockaddr *) &localAddr, sizeof(localAddr));
}

int system_network_send( int socket, const char* buffer, int len )
{
	return send(socket, buffer, len, 0);
}

int system_network_recv( int socket, char* buffer, int len )
{
	return recv(socket, buffer, len, 0);
}

int system_network_peek( int socket, char* buffer, int len )
{
	return recv(socket, buffer, len, MSG_PEEK);
}

int system_network_listen( int socket, int max_pending_connections )
{
	return listen(socket, max_pending_connections);
}


int system_network_can_read( int socket )
{
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(socket, &readSet);
	timeval timeout;
	timeout.tv_sec = 0;  // Zero timeout (poll)
	timeout.tv_usec = 0;

	select(socket, &readSet, NULL, NULL, &timeout);

	return FD_ISSET( socket, &readSet );
}

int system_network_can_write( int socket )
{
	fd_set writeSet;
	FD_ZERO(&writeSet);
	FD_SET(socket, &writeSet);
	timeval timeout;
	timeout.tv_sec = 0;  // Zero timeout (poll)
	timeout.tv_usec = 0;
	select(socket, NULL, &writeSet, NULL, &timeout);

	return FD_ISSET( socket, &writeSet );
}

int system_network_status( int socket, bool& can_write, bool& can_read, bool& has_error )
{
	fd_set writeSet, readSet, errorSet;

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_ZERO(&errorSet);

	FD_SET(socket, &readSet);
	FD_SET(socket, &writeSet);
	FD_SET(socket, &errorSet);

	timeval timeout;
	timeout.tv_sec = 0;  // Zero timeout (poll)
	timeout.tv_usec = 0;
	select(socket, NULL, &writeSet, NULL, &timeout);

	can_read  = FD_ISSET( socket, &readSet );
	can_write = FD_ISSET( socket, &writeSet );
	has_error = FD_ISSET( socket, &errorSet );

	return can_read << 2 | can_write << 1 | has_error;
}

int system_network_accept( int socket )
{
	struct sockaddr_in remoteAddr;
	int len = sizeof(remoteAddr);
	return accept(socket, (struct sockaddr *) &remoteAddr, &len);
}

int system_network_shutdown()
{
	return 1;
}

#endif
