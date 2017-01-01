/*************************************************************************************
Phyzaks
Copyright (c) 2012 John Rohrssen
Email: johnrohrssen@gmail.com
*************************************************************************************/

#ifndef PHYZAKS_PHYSICS_H_INCLUDED
#define PHYZAKS_PHYSICS_H_INCLUDED

#include "config.h"
#include "util.h"
#include "math_game.h"
#include "block.h"

//#define MAX_BUFFER_SIZE 32768
#define PHYSICS_OBJ( id, member ) s_physics.aRenderBlock.member[id]



int physics_initialize();
int physics_update(float fDt);
int physics_shutdown();

void physics_set_timemult( float fTimeMult);
void physics_set_gravity( const Vector4& vGravity);

int physics_create_object(int ownerObject, int assetid);

int physics_object_get_transform( int id, Matrix* pMat);
int physics_object_set_transform( int id, Matrix* pMat);

int physics_object_loaded( int id );

#endif
