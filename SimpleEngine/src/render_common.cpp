/*************************************************************************************
Phyzaks
Copyright (c) 2012 John Rohrssen
Email: johnrohrssen@gmail.com
*************************************************************************************/

#include "config.h"
#include "util.h"
#include "game_block.h"
#include "math_game.h"
#include "render.h"
#include "system.h"
#include "asset.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int render_build_frustum()
{
  PROFILE_FUNC();

  Matrix viewProjection;
  Vector4 plane;
#if 0
  // Left plane
  s_renderer.viewFrustum.left.x = viewProjection._41 + viewProjection._11;
  s_renderer.viewFrustum.left.y = viewProjection._42 + viewProjection._12;
  s_renderer.viewFrustum.left.z = viewProjection._43 + viewProjection._13;
  s_renderer.viewFrustum.left.w = viewProjection._44 + viewProjection._14;

  // Right plane
  s_renderer.viewFrustum.right.x = viewProjection._41 - viewProjection._11;
  s_renderer.viewFrustum.right.y = viewProjection._42 - viewProjection._12;
  s_renderer.viewFrustum.right.z = viewProjection._43 - viewProjection._13;
  s_renderer.viewFrustum.right.w = viewProjection._44 - viewProjection._14;

  // Top plane
  s_renderer.viewFrustum.top.x = viewProjection._41 - viewProjection._21;
  s_renderer.viewFrustum.top.y = viewProjection._42 - viewProjection._22;
  s_renderer.viewFrustum.top.z = viewProjection._43 - viewProjection._23;
  s_renderer.viewFrustum.top.w = viewProjection._44 - viewProjection._24;

  // Bottom plane
  s_renderer.viewFrustum.bottom.x = viewProjection._41 + viewProjection._21;
  s_renderer.viewFrustum.bottom.y = viewProjection._42 + viewProjection._22;
  s_renderer.viewFrustum.bottom.z = viewProjection._43 + viewProjection._23;
  s_renderer.viewFrustum.bottom.w = viewProjection._44 + viewProjection._24;

  // Near plane
  s_renderer.viewFrustum.front.x = viewProjection._31;
  s_renderer.viewFrustum.front.y = viewProjection._32;
  s_renderer.viewFrustum.front.z = viewProjection._33;
  s_renderer.viewFrustum.front.w = viewProjection._34;

  // Far plane
  s_renderer.viewFrustum.back.x = viewProjection._41 - viewProjection._31;
  s_renderer.viewFrustum.back.y = viewProjection._42 - viewProjection._32;
  s_renderer.viewFrustum.back.z = viewProjection._43 - viewProjection._33;
  s_renderer.viewFrustum.back.w = viewProjection._44 - viewProjection._34;

  // old
  // Left plane
  s_renderer.viewFrustum.left.x = viewProjection._14 + viewProjection._11;
  s_renderer.viewFrustum.left.y = viewProjection._24 + viewProjection._21;
  s_renderer.viewFrustum.left.z = viewProjection._34 + viewProjection._31;
  s_renderer.viewFrustum.left.w = viewProjection._44 + viewProjection._41;

  // Right plane
  s_renderer.viewFrustum.right.x = viewProjection._14 - viewProjection._11;
  s_renderer.viewFrustum.right.y = viewProjection._24 - viewProjection._21;
  s_renderer.viewFrustum.right.z = viewProjection._34 - viewProjection._31;
  s_renderer.viewFrustum.right.w = viewProjection._44 - viewProjection._41;

  // Top plane
  s_renderer.viewFrustum.top.x = viewProjection._14 - viewProjection._12;
  s_renderer.viewFrustum.top.y = viewProjection._24 - viewProjection._22;
  s_renderer.viewFrustum.top.z = viewProjection._34 - viewProjection._32;
  s_renderer.viewFrustum.top.w = viewProjection._44 - viewProjection._42;

  // Bottom plane
  s_renderer.viewFrustum.bottom.x = viewProjection._14 + viewProjection._12;
  s_renderer.viewFrustum.bottom.y = viewProjection._24 + viewProjection._22;
  s_renderer.viewFrustum.bottom.z = viewProjection._34 + viewProjection._32;
  s_renderer.viewFrustum.bottom.w = viewProjection._44 + viewProjection._42;

  // Near plane
  s_renderer.viewFrustum.front.x = viewProjection._13;
  s_renderer.viewFrustum.front.y = viewProjection._23;
  s_renderer.viewFrustum.front.z = viewProjection._33;
  s_renderer.viewFrustum.front.w = viewProjection._43;

  // Far plane
  s_renderer.viewFrustum.back.x = viewProjection._14 - viewProjection._13;
  s_renderer.viewFrustum.back.y = viewProjection._24 - viewProjection._23;
  s_renderer.viewFrustum.back.z = viewProjection._34 - viewProjection._33;
  s_renderer.viewFrustum.back.w = viewProjection._44 - viewProjection._43;
#endif	

#if USE_XNAMATH
  viewProjection = XMMatrixMultiply(s_renderer.viewMatrix, s_renderer.projMatrix );

  plane = XMLoadFloat4(&s_renderer.viewFrustum.top);
  XMStoreFloat4( &s_renderer.viewFrustum.top,    XMPlaneNormalize( plane ));

  plane = XMLoadFloat4(&s_renderer.viewFrustum.bottom);
  XMStoreFloat4( &s_renderer.viewFrustum.bottom, XMPlaneNormalize( plane ));



  plane = XMLoadFloat4(&s_renderer.viewFrustum.back);
  XMStoreFloat4( &s_renderer.viewFrustum.back,   XMPlaneNormalize( plane ));

  plane = XMLoadFloat4(&s_renderer.viewFrustum.front);
  XMStoreFloat4( &s_renderer.viewFrustum.front,  XMPlaneNormalize( plane ));


  plane = XMLoadFloat4(&s_renderer.viewFrustum.left);
  XMStoreFloat4( &s_renderer.viewFrustum.left,   XMPlaneNormalize( plane ));

  plane = XMLoadFloat4(&s_renderer.viewFrustum.right);
  XMStoreFloat4( &s_renderer.viewFrustum.right,  XMPlaneNormalize( plane ));
#else
  multiply( &viewProjection, &s_renderer.viewMatrix, &s_renderer.projMatrix);

  add( &s_renderer.viewFrustum.left, &viewProjection.m[3], &viewProjection.m[0] );
  sub( &s_renderer.viewFrustum.right, &viewProjection.m[3], &viewProjection.m[0] );

  sub( &s_renderer.viewFrustum.top, &viewProjection.m[3], &viewProjection.m[1] );
  add( &s_renderer.viewFrustum.bottom, &viewProjection.m[3], &viewProjection.m[1] );

  s_renderer.viewFrustum.front = viewProjection.m[2];

  sub( &s_renderer.viewFrustum.back, &viewProjection.m[3], &viewProjection.m[2] );

  plane_normalize( &s_renderer.viewFrustum.top );
  plane_normalize( &s_renderer.viewFrustum.bottom );

  plane_normalize( &s_renderer.viewFrustum.left );
  plane_normalize( &s_renderer.viewFrustum.right );

  plane_normalize( &s_renderer.viewFrustum.front );
  plane_normalize( &s_renderer.viewFrustum.back );

#endif
  return 1;
}

int render_cull_scene()
{
  PROFILE_FUNC();
  //float fStart = system_get_time_secs(); 

  // this code handles spheres that are fully contained within the view frustum
  Vector4 pos;
  float fDot = 0.0f;
  int res = 0;
  for( int i=1; i < s_renderer.aRenderBlock.iTail; ++i )
  {
    float fRadius = s_renderer.aRenderBlock.fRadius[i];

    // 0x80000000
    pos = s_renderer.aRenderBlock.aWorldMatrix[i].m[3];

    fDot = dot( &s_renderer.viewFrustum.front, &pos );
    fDot += fRadius + s_renderer.viewFrustum.front.w;
    res = FLOAT_GET_SIGN( fDot );

    fDot = dot( &s_renderer.viewFrustum.back, &pos );
    fDot += fRadius + s_renderer.viewFrustum.back.w;
    res |= FLOAT_GET_SIGN( fDot );

    fDot = dot( &s_renderer.viewFrustum.top, &pos );
    fDot += fRadius + s_renderer.viewFrustum.top.w;
    res |= FLOAT_GET_SIGN( fDot );

    fDot = dot( &s_renderer.viewFrustum.bottom, &pos );
    fDot += fRadius + s_renderer.viewFrustum.bottom.w;
    res |= FLOAT_GET_SIGN( fDot );

    fDot = dot( &s_renderer.viewFrustum.left, &pos );
    fDot += fRadius + s_renderer.viewFrustum.left.w;
    res |= FLOAT_GET_SIGN( fDot );

    fDot = dot( &s_renderer.viewFrustum.right, &pos );
    fDot += fRadius + s_renderer.viewFrustum.right.w;
    res |= FLOAT_GET_SIGN( fDot );

    s_renderer.aRenderBlock.uVisible[i] = res;
  }

  //system_log( "cull ms=%f\n", (system_get_time_secs()-fStart) *1000.0f);
  return 1;
}

int render_draw()
{
  PROFILE_FUNC();

  s_renderer.iProcessedAssets = 0;
  // cull all render objects in the scene
  render_cull_scene();

  float fStart = system_get_time_secs();

  uint32 shader = -1;
  uint32 mesh = -1;

  s_renderer.iVisibleObjects = 0;

  Matrix temp = matrix_identity();
  matrix_rotation_z(&temp, 0.5f );
  multiply( &s_renderer.vSunPos, &temp, s_renderer.vSunPos);

  for( int i=1; i < s_renderer.aRenderBlock.iTail; ++i )
  {
    PROFILE_BLOCK( render_one_object );

    s_renderer.iVisibleObjects += render_draw_object( i, shader, mesh );
    //normalMatrix = XMMatrixTranspose(XMMatrixInverse(&temp, XMMatrixMultiply(s_renderer.aRenderBlock.aWorldMatrix[i], s_renderer.viewMatrix)));
  }

  //system_log( "render ms=%f, v=%d\n", (system_get_time_secs()-fStart) *1000.0f,s_renderer.iVisibleObjects);

  return 1;
}

Matrix* render_view_matrix()
{
  return &s_renderer.viewMatrix;
}

void render_set_view_matrix(Matrix* mat)
{
  s_renderer.viewMatrix = *mat;
}

int render_ui_update(float fDt)
{
  return 1;
}

