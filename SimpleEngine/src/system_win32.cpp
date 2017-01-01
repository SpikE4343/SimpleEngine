/*************************************************************************************
  Phyzaks
  Copyright (c) 2012 John Rohrssen
  Email: johnrohrssen@gmail.com
*************************************************************************************/

#include "config.h"

#if SYSTEM_WIN32
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <windows.h>
#include <winsock.h>

#include "system.h"
#include "math_game.h"
#include "game.h"

#define NUM_KEYS 256
#define NUM_MOUSE_BUTTONS 4

struct System
{
	HWND   hWindow;
	HDC  hDeviceContext;

	bool aInputKeyMap[NUM_KEYS];
	bool aMouseButtons[NUM_MOUSE_BUTTONS];

	int iMouseX;
	int iMouseY;

	int iMouseLastX;
	int iMouseLastY;

	float fFrameTimeSecs;
	float fStartSysTimeSecs;
	float fLastSysTimeSecs;
	float fClockFreq;

	WSADATA wsaData;
};

static System s_system;

int system_initialize(void* pNativeData)
{
	s_system.hWindow = 0;
	s_system.hDeviceContext = 0;

	memset( s_system.aInputKeyMap,  0, sizeof(bool) * NUM_KEYS );
	memset( s_system.aMouseButtons, 0, sizeof(bool) * NUM_MOUSE_BUTTONS );

	s_system.iMouseX = 0;
	s_system.iMouseY = 0;

	s_system.iMouseLastX = 0;
	s_system.iMouseLastY = 0;

	LARGE_INTEGER qwfreq;
	QueryPerformanceFrequency(&qwfreq);

	s_system.fClockFreq = (float)qwfreq.QuadPart;
	s_system.fStartSysTimeSecs = 0;

	s_system.fStartSysTimeSecs = s_system.fLastSysTimeSecs = system_get_time_secs();
	s_system.fFrameTimeSecs = 1;

	return 1;
}

int system_update()
{
	PROFILE_FUNC();
	float fNow = system_get_time_secs();
	s_system.fFrameTimeSecs = (fNow - s_system.fLastSysTimeSecs);
	s_system.fLastSysTimeSecs = fNow;

	s_system.iMouseLastX = s_system.iMouseX;
	s_system.iMouseLastY = s_system.iMouseY;

	MSG msg = { 0 };
    if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0 )
    {
		PROFILE_BLOCK( process_msg );
        if (msg.message==WM_QUIT)
            return 0;
            
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    }

	if( s_system.hWindow == NULL )
	{
		game_begin_initialize();
	}

	return 1;
}

int system_shutdown()
{
	return 1;
}

LRESULT WINAPI system_windowProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) 
{
    LRESULT  lRet = 1; 
    POINT      point;
    GetCursorPos( &point );
    ScreenToClient( hWnd, &point );

	ShowCursor( true );
    switch (uMsg) 
    { 
        case WM_CREATE:
            break;

        case WM_DESTROY:
            PostQuitMessage(0);             
            break; 

        case WM_KEYDOWN:
			s_system.aInputKeyMap[ wParam ] = true;
            break;

        case WM_KEYUP:
			s_system.aInputKeyMap[ wParam ] = false;
            break;
            
        case WM_LBUTTONDOWN:
            ShowCursor( false );
			s_system.aMouseButtons[Input::LEFT_BUTTON] = true;
			break;

        case WM_LBUTTONUP:
            while( ShowCursor( true ) < 0 );
			s_system.aMouseButtons[Input::LEFT_BUTTON] = false;
            break;

		case WM_RBUTTONDOWN:
            ShowCursor( false );
			s_system.aMouseButtons[Input::RIGHT_BUTTON] = true;
			break;

        case WM_RBUTTONUP:
            while( ShowCursor( true ) < 0 );
			s_system.aMouseButtons[Input::RIGHT_BUTTON] = false;
            break;
            
        case WM_MOUSEMOVE:
			s_system.iMouseX = point.x;
			s_system.iMouseY = point.y;
            break;

        default: 
            lRet = DefWindowProc (hWnd, uMsg, wParam, lParam); 
            break; 
    } 

    return lRet; 
}

int system_createWindow ( int width, int height, const char *title )
{
    WNDCLASS wndclass = {0}; 
    DWORD    wStyle   = 0;
    RECT     windowRect;
    HINSTANCE hInstance = GetModuleHandle(NULL);
    HWND     hWnd=NULL;


    wndclass.style         = CS_OWNDC;
    wndclass.lpfnWndProc   = (WNDPROC)system_windowProc; 
    wndclass.hInstance     = hInstance; 
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); 
    wndclass.lpszClassName = "opengles2.0"; 

    if (!RegisterClass (&wndclass) ) 
        return 0; 

    wStyle = WS_VISIBLE | WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION;
       
    // Adjust the window rectangle so that the client area has
    // the correct number of pixels
    windowRect.left = 0;
    windowRect.top = 0;
    windowRect.right = width;
    windowRect.bottom = height;

    AdjustWindowRect ( &windowRect, wStyle, FALSE );

    

    hWnd = CreateWindow(
                            "opengles2.0",
                            title,
                            wStyle,
                            1440/2 - ( width/2),
                            900/2 - ( height/2),
                            windowRect.right - windowRect.left,
                            windowRect.bottom - windowRect.top,
                            NULL,
                            NULL,
                            hInstance,
                            NULL);

    if ( hWnd == NULL )
        return NULL;

    ShowWindow ( hWnd, TRUE );

	s_system.hWindow = hWnd;
	s_system.hDeviceContext = GetDC( hWnd );

    return 1;
}

int system_display_created( int iFormat )
{
	return 1;
}

void* system_display()
{
	return &s_system.hDeviceContext;
}

void* system_window()
{
	return &s_system.hWindow;
}

// input management

bool system_isKeyDown(int iKey)
{
	return s_system.aInputKeyMap[ iKey ];
}

bool system_mouse_button_down(int iButton)
{
	return s_system.aMouseButtons[iButton];
}

int system_mouse_delta_x()
{
	return s_system.iMouseLastX - s_system.iMouseX;
}

int system_mouse_delta_y()
{
	return s_system.iMouseLastY - s_system.iMouseY;
}

float system_get_frame_time_secs()
{
	return s_system.fFrameTimeSecs;
}

float system_get_time_secs()
{
	LARGE_INTEGER qwNow;
	QueryPerformanceCounter(&qwNow);

	//printf( "now=%I64u, freq=%f, time=%f\n",qwNow.QuadPart, s_system.fClockFreq, ((double)qwNow.QuadPart/(double)s_system.fClockFreq) );
	//return (float) (((double) (time.tv_sec * 1000000.0 + time.tv_usec) / (double) s_system.fClockFreq) - s_system.fStartSysTimeSecs);

	return (float)(((double)qwNow.QuadPart/(double)s_system.fClockFreq) - s_system.fStartSysTimeSecs);
}

void system_exit(int error)
{
	exit(error);
}

void system_sleep( int ms )
{
	Sleep(ms);
}

// Asset manager

void system_asset_dump_dir(const char* pDir)
{

}

system_asset* system_asset_open(const char* sFile, SystemAssetOpenMode iMode)
{
	verify( sFile != NULL);
	char* mode = "r";
	switch( iMode )
	{
	case SOM_READ:
		mode = "rb";
		break;

	case SOM_READ_TEXT:
		mode = "r";
		break;

	case SOM_READ_TEXT_APPEND:
		mode = "r+";
		break;

	case SOM_READ_WRITE:
		mode = "r+b";
		break;

	case SOM_READ_WRITE_APPEND:
		mode = "a+b";
		break;

	case SOM_READ_WRITE_TEXT:
		mode = "r+b";
		break;

	case SOM_READ_WRITE_TEXT_APPEND:
		mode = "a+";
		break;

	case SOM_WRITE:
		mode = "wb";
		break;

	case SOM_WRITE_APPEND:
		mode = "ab";
		break;

	case SOM_WRITE_TEXT:
		mode = "w";
		break;

	case SOM_WRITE_TEXT_APPEND:
		mode = "a";
		break;
	}

	system_asset* asset = fopen(sFile, mode);

	
	debug_log( "system_asset_open: file=%s, asset=%x, error=%s", sFile, asset, strerror(errno) );
	return asset;
}

int           system_asset_gets  		   ( system_asset* asset, char* str, int max )
{
	return fgets( str, max, asset) != NULL;
}

int system_asset_read(system_asset* asset, char* buffer, int max_length )
{
	return fread( buffer, 1, max_length, asset );
}

int system_asset_write(system_asset* asset, char* buffer, int length)
{
	return fwrite( buffer, 1, length, asset );
}

int system_asset_length(system_asset* asset)
{
	fpos_t pos, oldpos;
	fgetpos(asset, &oldpos );
	fseek( asset, 0, SEEK_END);
	fgetpos(asset, &pos );
	fsetpos(asset, &oldpos );

	debug_log( "asset %x, size: %d", asset, (int)pos );
	return (int)pos;
}

int system_asset_bytes_remaining( system_asset* asset )
{
	int pos = system_asset_getpos(asset);
	return system_asset_length(asset) - pos;
}

void system_asset_close(system_asset* asset)
{
	debug_log( "asset closed file: %x", asset );
	fclose(asset);
}

const void* system_asset_buffer(system_asset* asset)
{
	return NULL;
}

int system_asset_getpos( system_asset* asset )
{
	fpos_t pos;
	fgetpos(asset, &pos );
	return (int)pos;
}

int system_asset_setpos ( system_asset* asset, int pos )
{
	fpos_t fpos = pos;
	fsetpos(asset, &fpos );
	return 0;
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
	
}


const Vector4& system_get_accelerometer()
{
	return Vector4::ZERO;
}

struct system_thread_startdata
{
	system_thread_call func;
	void* data;
};

int system_thread_starter(void* start_data)
{
	system_thread_startdata* data = (system_thread_startdata*)start_data;
	system_thread_call func = data->func;
	int ret = (func)(data->data);

	delete data;

	return ret;
}

int system_create_thread(system_thread_call func, void* data)
{
	system_thread_startdata* start_data = new system_thread_startdata;
	start_data->func = func;
	start_data->data = data;
	int iThreadId=-1;
	if( CreateThread(
	            NULL,                   // default security attributes
	            0,                      // use default stack size
	            (LPTHREAD_START_ROUTINE)system_thread_starter,       // thread function name
	            start_data,          // argument to thread function
	            0,                      // use default creation flags
	            (LPDWORD)&iThreadId) == NULL )
	{
		return -1;
	}

	return iThreadId;
}

int system_current_thread()
{
	return (int)GetCurrentThreadId();
}

#if 0
WSADATA wsaData;                 /* Structure for WinSock setup communication */

if (argc != 2)     /* Test for correct number of arguments */
{
    fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
    exit(1);
}

echoServPort = atoi(argv[1]);  /* first arg:  Local port */

if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) /* Load Winsock 2.0 DLL */
{
    fprintf(stderr, "WSAStartup() failed");
    exit(1);
}

/* Create socket for incoming connections */
if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithError("socket() failed");

/* Construct local address structure */
memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
echoServAddr.sin_family = AF_INET;                /* Internet address family */
echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
echoServAddr.sin_port = htons(echoServPort);      /* Local port */
#endif

// networking
int system_network_initialize()
{
	if (WSAStartup(MAKEWORD(2, 0), &s_system.wsaData) != 0) 
	{
		system_log( "Unable to initialize winsock interface");
		return 0;
	}

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
	return closesocket(socket);
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
	select(socket, NULL, &writeSet, NULL, &timeout) == 1;

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
	return WSACleanup();
}

#endif
