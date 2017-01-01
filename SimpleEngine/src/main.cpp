/*************************************************************************************
  Phyzaks
  Copyright (c) 2012 John Rohrssen
  Email: johnrohrssen@gmail.com
*************************************************************************************/

#include "config.h"
#include "system.h"
#include "game.h"
#include "client.h"
#include "network.h"

#if SYSTEM_ANDROID 

#include <android_native_app_glue.h>

void android_main(struct android_app* state) 
{
	system_initialize((void*)state);

	game_run(NULL);

	system_shutdown();
}

#elif SYSTEM_WIN32

#include <windows.h>

int main()
{
	system_initialize(NULL);
	system_network_initialize();
	client_initialize("127.0.0.1", SERVER_CLIENT_PORT);

	game_run(NULL);

	client_shutdown();
	system_network_shutdown();
	system_shutdown();

	return 0;
}

#elif SYSTEM_WIN32_DLL

#include <Windows.h>
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#elif SYSTEM_WIN32_QT_TOOL

#include "math_game.h"
#include "render.h"
#include "physics.h"

int main(int argc, char *argv[])
{
    system_initialize(NULL);

    int error = 0;

    system_log( "TOOL initialize" );

    error = system_createWindow( 1024, 768, "PhyzaksBuilder" );
    if( error != 1 )
    {
        system_log( "system_createWindow failed: %d", error);
        return 0;
    }

    error = render_initialize(1024,768);
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

    while(true)
    {
        PROFILE_FUNC();
        if( system_update() == 0 )
            break;

        if( system_isKeyDown( Input::K_ESCAPE ) )
            break;

        float fDt = system_get_frame_time_secs();

        // update physics data
        physics_update(fDt);

        float fSpeed = 1.0f * fDt;

        Matrix* pCameraMat = render_view_matrix();
        Matrix tempMat;


        if( system_mouse_button_down( Input::RIGHT_BUTTON ) )
        {
            matrix_rotation_y( &tempMat, (float)system_mouse_delta_x()*0.01f );
            multiply( pCameraMat, &tempMat );

            matrix_rotation_x( &tempMat, (float)system_mouse_delta_y()*0.01f );
            multiply( pCameraMat, &tempMat );
        }

        game_update_objects(fDt);

        render_beginframe();

        render_draw();

        render_endframe();
    }

    system_shutdown();
}



#endif
