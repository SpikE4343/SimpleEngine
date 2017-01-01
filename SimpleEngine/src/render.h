#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include "config.h"
#include "util.h"
#include "math_game.h"
#include "block.h"

#define MAX_BUFFER_SIZE 4
#define RENDER_OBJ( id, member ) aRenderBlock.member[id]

struct Shader
{

};


enum RenderObjectFlags
{
	RV_EMPTY = 0,
	RV_ACTIVE = 1,
	RV_VISIBLE = 2
};

enum ModelLoadSection
{
	LS_HEADER,
	LS_VERTICES,
	LS_INDICES,
	LS_UVS,
	LS_VERTEX_NORMALS
};

struct RenderObjectBlock
{
	int iTail;

	MEMBER( int, uFlags );
	MEMBER( int, uVisible );
	MEMBER( float, fRadius );
	MEMBER( int, uShader );
	MEMBER( int, uMesh );
	MEMBER( Matrix, aWorldMatrix );
};

struct Stats
{
	int iVisibleObjects;
	float fFrameTime;
};

class RendererData
{
public:
	int iWidth;
	int iHeight;
	int iVisibleObjects;
  int iFrameCount;

	Matrix viewMatrix;
	Matrix viewDebugMatrix;
	Matrix projMatrix;

	float fFarPlane;
	float fNearPlane;
	float fFOV;
	float fScreenRatio;

	Frustum viewFrustum;

	Vector4 vSunPos;
	Vector4 vSunColor;

	RenderObjectBlock aRenderBlock;

	//char pTempBuffer[MAX_BUFFER_SIZE];
};

int render_initialize(int iWidth, int iHeight);
int render_init_font();
int render_begin_frame();
int render_draw();
int render_end_frame();
int render_shutdown();

int render_draw_text( const char* pText, int x, int y );

int render_clone_renderobject( int existing, Vector4 pos );
//int render_load_game_model( const char* pFileName );
//RendererData& render_data();

//unsigned int render_load_render_shader ( const char *fileName );

int render_draw_object(int iObj, uint& iCurrentShader, uint& iCurrentMesh);

//int render_load_texture( const char* pFileName, int shader );
//bool render_load_tga( const char *fileName, char **buffer, int *width, int *height );

Matrix* render_view_matrix();

#endif
