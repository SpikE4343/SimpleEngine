/*
 * mesh.h
 *
 *  Created on: Dec 4, 2011
 *      Author: John
 */

#ifndef RENDER_MESH_H_
#define RENDER_MESH_H_

struct game_mesh_header
{
	short iIndexBufferSize;
	short iVertexBufferSize;
	short iVertexNormalBufferSize;
	short iTexturCoordinateBufferSize;
	short data_start;
};

struct game_mesh_data
{
	float* pVertexBuffer;
	float* pVertexNormalBuffer;
	float* pTextureCoordinateBuffer;
	short* pIndexBuffer;
};

struct game_mesh
{
	uint32 uId;
	game_mesh_header mesh_info;
	game_mesh_data   mesh_data;

	float* pVertexBuffer;
	float* pVertexNormalBuffer;
	float* pTextureCoordinateBuffer;
	short* pIndexBuffer;

	void load(game_mesh_header* header)
	{
		mesh_info = *header;
		mesh_data.pVertexBuffer            = (float*)&mesh_info.data_start;
		mesh_data.pVertexNormalBuffer      = (float*)mesh_data.pVertexBuffer       + mesh_info.iVertexBufferSize;
		mesh_data.pIndexBuffer             = (short*)mesh_data.pVertexNormalBuffer + mesh_info.iVertexNormalBufferSize;
		mesh_data.pTextureCoordinateBuffer = (short*)mesh_data.pIndexBuffer        + mesh_info.iIndexBufferSize;
	}

	void load_text(game_mesh_header* header);
	void save( const char* sFile );
};

struct game_shader_params
{
	short iType;
	short iGameBinding;
	const char* pName;
};

struct game_shader_attributes
{
	short iType;
	short iIndex;
	const char* pName;
};

struct game_shader
{
	uint32 uId;
	game_shader_attributes aAttributes[8];
	short iNumAttributes;

	game_shader_params     aParams[32];
	short iNumParams;

	const char* pVertexProgramSrc;
	const char* pFragmentProgramSrc;
};

struct game_texture
{
	uint32 uId;
	int iWidth;
	int iHeight;
	int iColorDepth;

	char* pData;
};


void load_game_shader_text( const char* pFile, game_shader& shader );
void load_game_shader_baked( const char* pFile, game_shader& shader );

#endif /* MESH_H_ */
