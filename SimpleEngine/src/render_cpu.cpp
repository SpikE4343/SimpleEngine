#include "config.h"
#include "block.h"
#include "math_game.h"
#include "render.h"

//#if USE_ARDUINO_RENDERER

void render_draw_mesh(const Matrix& modelViewProjection, int mesh );

struct ShaderBlock
{
  int iTail;

  // shader bindings
  MEMBER( uint, uProgram           );
  MEMBER( uint, uWorldMatrixParam  );
  MEMBER( uint, uViewMatrixParam   );
  MEMBER( uint, uProjMatrixParam   );
  //MEMBER( uint, uNormalMatrixParam );
  //MEMBER( uint, uFinalTransformParam );
  //MEMBER( uint, uLightPosParam     );
  //MEMBER( uint, uLightColorParam   );
  MEMBER( uint, uTexture0          );
  MEMBER( uint, uTexture0Loc       );
  //MEMBER( uint, uSunPos            );
  //MEMBER( uint, uSunColor          );
};

struct MeshDataBlock
{
  int iTail;

  MEMBER( Vector4*, uVertexBuffer);
  MEMBER( uint16, uNumVerts );

  //MEMBER( uint, uVertexNormalBuffer);
  //MEMBER( uint, uNumVertNormals );

  MEMBER( uint16*, uIndexBuffer);
  MEMBER( uint16, uNumIndices );

  MEMBER( uint16*, uTexCoordBuffer);
  MEMBER( uint16, uNumTexCoords );

  MEMBER( int, iAssetId );
};

struct TextureBlock
{
  int iTail;
  MEMBER( uint, uTexture);
  MEMBER( uint, uWidth);
  MEMBER( uint, uHeight);
};

struct Color 
{
  uint8 r, g, b;
};

struct CPURenderer : RendererData
{
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
  ShaderBlock aShaderBlock;
  MeshDataBlock aMeshBlock;
  TextureBlock aTextureBlock;

  Color aFrameBuffer[];

};

CPURenderer s_cpu_renderer;

// Renderer
int RenderObjectBlockSize( RenderObjectBlock* pBlock )
{
  return pBlock->iTail + 1;
}

int RenderObjectBlockRemove( RenderObjectBlock* pBlock, int index )
{
  if( pBlock->iTail > 1 )
  {
    REMOVE( pBlock, index, uFlags );
    REMOVE( pBlock, index, uVisible );
    REMOVE( pBlock, index, aWorldMatrix );

    REMOVE( pBlock, index, fRadius );

    // shader bindings
    REMOVE( pBlock, index, uShader );
    REMOVE( pBlock, index, uMesh );

    --pBlock->iTail;
  }

  return 1;
}

int render_create_renderobject()
{
  // to many objects in the block
  if( s_renderer.aRenderBlock.iTail - 1 >= OBJECT_BLOCK_SIZE )
    return -1;

  int index = s_renderer.aRenderBlock.iTail++;

  s_renderer.aRenderBlock.uFlags[index] = 1;
  s_renderer.aRenderBlock.uVisible[index] = 0;
  s_renderer.aRenderBlock.fRadius[index] = 1.0f;

  s_renderer.aRenderBlock.aWorldMatrix[index] = Matrix::IDENTITY;

  return index;
}

int render_clone_renderobject(int existing, Vector4 pos)
{
  // to many objects in the block
  if( s_renderer.aRenderBlock.iTail - 1 >= OBJECT_BLOCK_SIZE )
    return -1;

  int index = s_renderer.aRenderBlock.iTail++;

  RENDER_OBJ( index, uFlags )   = 1;
  RENDER_OBJ( index, uVisible ) = 0;
  RENDER_OBJ( index, fRadius )  = 1.0f;

  RENDER_OBJ( index, aWorldMatrix ) = matrix_identity();

  RENDER_OBJ( index, uShader ) = RENDER_OBJ( existing, uShader );
  RENDER_OBJ( index, uMesh )   = RENDER_OBJ( existing, uMesh );

  RENDER_OBJ( index, aWorldMatrix ).m[3] = pos;
  return index;
}

RendererData& render_data()
{
  return s_cpu_renderer;
}

// ======================================
int render_create_shader()
{
  // to many objects in the block
  if( s_cpu_renderer.aShaderBlock.iTail < 0 || 
    s_cpu_renderer.aShaderBlock.iTail - 1 >= OBJECT_BLOCK_SIZE )
    return -1;

  int index = s_cpu_renderer.aShaderBlock.iTail++;
  return index;
}

int render_create_mesh()
{
  //  // to many objects in the block
  //  if( aMeshBlock.iTail - 1 >= OBJECT_BLOCK_SIZE )
  //    return -1;
  //
  //  int index = aMeshBlock.iTail++;
  //
  //  aMeshBlock.uVertexBuffer[index] = 0;
  //  aMeshBlock.uIndexBuffer[index]  = 0;
  //  return index;
  return 1;
}

int render_init_font()
{

  return 1;
}



Matrix* render_view_matrix()
{
  return &s_renderer.viewMatrix;
}

int render_build_frustum()
{
  PROFILE_FUNC();

  Matrix viewProjection;
  Vector4 plane;
#if 0
  // Left plane
  viewFrustum.left.x = viewProjection._41 + viewProjection._11;
  viewFrustum.left.y = viewProjection._42 + viewProjection._12;
  viewFrustum.left.z = viewProjection._43 + viewProjection._13;
  viewFrustum.left.w = viewProjection._44 + viewProjection._14;

  // Right plane
  viewFrustum.right.x = viewProjection._41 - viewProjection._11;
  viewFrustum.right.y = viewProjection._42 - viewProjection._12;
  viewFrustum.right.z = viewProjection._43 - viewProjection._13;
  viewFrustum.right.w = viewProjection._44 - viewProjection._14;

  // Top plane
  viewFrustum.top.x = viewProjection._41 - viewProjection._21;
  viewFrustum.top.y = viewProjection._42 - viewProjection._22;
  viewFrustum.top.z = viewProjection._43 - viewProjection._23;
  viewFrustum.top.w = viewProjection._44 - viewProjection._24;

  // Bottom plane
  viewFrustum.bottom.x = viewProjection._41 + viewProjection._21;
  viewFrustum.bottom.y = viewProjection._42 + viewProjection._22;
  viewFrustum.bottom.z = viewProjection._43 + viewProjection._23;
  viewFrustum.bottom.w = viewProjection._44 + viewProjection._24;

  // Near plane
  viewFrustum.front.x = viewProjection._31;
  viewFrustum.front.y = viewProjection._32;
  viewFrustum.front.z = viewProjection._33;
  viewFrustum.front.w = viewProjection._34;

  // Far plane
  viewFrustum.back.x = viewProjection._41 - viewProjection._31;
  viewFrustum.back.y = viewProjection._42 - viewProjection._32;
  viewFrustum.back.z = viewProjection._43 - viewProjection._33;
  viewFrustum.back.w = viewProjection._44 - viewProjection._34;

  // old
  // Left plane
  viewFrustum.left.x = viewProjection._14 + viewProjection._11;
  viewFrustum.left.y = viewProjection._24 + viewProjection._21;
  viewFrustum.left.z = viewProjection._34 + viewProjection._31;
  viewFrustum.left.w = viewProjection._44 + viewProjection._41;

  // Right plane
  viewFrustum.right.x = viewProjection._14 - viewProjection._11;
  viewFrustum.right.y = viewProjection._24 - viewProjection._21;
  viewFrustum.right.z = viewProjection._34 - viewProjection._31;
  viewFrustum.right.w = viewProjection._44 - viewProjection._41;

  // Top plane
  viewFrustum.top.x = viewProjection._14 - viewProjection._12;
  viewFrustum.top.y = viewProjection._24 - viewProjection._22;
  viewFrustum.top.z = viewProjection._34 - viewProjection._32;
  viewFrustum.top.w = viewProjection._44 - viewProjection._42;

  // Bottom plane
  viewFrustum.bottom.x = viewProjection._14 + viewProjection._12;
  viewFrustum.bottom.y = viewProjection._24 + viewProjection._22;
  viewFrustum.bottom.z = viewProjection._34 + viewProjection._32;
  viewFrustum.bottom.w = viewProjection._44 + viewProjection._42;

  // Near plane
  viewFrustum.front.x = viewProjection._13;
  viewFrustum.front.y = viewProjection._23;
  viewFrustum.front.z = viewProjection._33;
  viewFrustum.front.w = viewProjection._43;

  // Far plane
  viewFrustum.back.x = viewProjection._14 - viewProjection._13;
  viewFrustum.back.y = viewProjection._24 - viewProjection._23;
  viewFrustum.back.z = viewProjection._34 - viewProjection._33;
  viewFrustum.back.w = viewProjection._44 - viewProjection._43;
#endif  

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

  for( int i=0; i < s_renderer.aRenderBlock.iTail; ++i )
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

  // cull all render objects in the scene
  //render_cull_scene();

  return 1;
}

int render_draw_object(int iObj, uint& iCurrentShader, uint& iCurrentMesh)
{
  if( !s_renderer.aRenderBlock.uVisible[iObj] )
    return 0;

  Matrix mvp = matrix_identity();
  multiply( &mvp, &s_renderer.aRenderBlock.aWorldMatrix[iObj], &s_renderer.viewMatrix );

  Matrix tempNormalMat = matrix_identity();
  matrix_inverse_nonui(&tempNormalMat, &mvp );
  transpose( &tempNormalMat );

  multiply( &mvp, &s_renderer.projMatrix );

  render_draw_mesh( mvp, iCurrentMesh );

  //system_log( "drawing %d", iObj);

  return 1;
}


int render_initialize(int width, int height)
{
  s_renderer.iWidth = width;
  s_renderer.iHeight = height;


  s_renderer.fFarPlane = 20000.0f;
  s_renderer.fNearPlane = 1.0f;
  s_renderer.fFOV = 55.0f;

  //vector4_set( &vSunPos, 0.0f, 100000.0f, 1000000.0f, 1.0f );
  //vector4_set( &vSunColor, 1.0f, 0.0f, 0.0f, 1.0f );

  //viewDebugMatrix = matrix_identity();

  //viewMatrix = Matrix::IDENTITY;
  //vector4_set( &viewMatrix.m[3], 0.0f, 0.0f, -20.0f, 1.0f );

  //  aRenderBlock.iTail  = 0;
  //  aMeshBlock.iTail    = 0;
  //  aShaderBlock.iTail  = 0;
  //  aTextureBlock.iTail = 0;

  s_renderer.fScreenRatio = s_renderer.iWidth / (float)s_renderer.iHeight;

  matrix_create_perspective( 
    &s_renderer.projMatrix, 
    s_renderer.fFOV , 
    s_renderer.fScreenRatio, 
    s_renderer.fNearPlane, 
    s_renderer.fFarPlane
    );

  render_build_frustum();

  return 1;
}

int render_begin_frame()
{
  // TODO: cache until fov changes
  s_renderer.projMatrix = matrix_identity();
  matrix_create_perspective( 
    &s_renderer.projMatrix, 
    s_renderer.fFOV, 
    s_renderer.fScreenRatio, 
    s_renderer.fNearPlane, 
    s_renderer.fFarPlane
    );

  render_build_frustum();



  return 1;
}

int render_draw_text( const char* pText, int x, int y )
{

  return 1;
}

int render_end_frame()
{
  PROFILE_FUNC();

  ++s_renderer.iFrameCount;
  //eglSwapBuffers ( eglDisplay, eglSurface );
  //checkGlError("eglSwapBuffers");
  //tv.delay(1000);
  return 1;
}

int render_shutdown()
{
  return 1;
}

void render_apply_vertex_shader(const Matrix& modelViewProjection, int index, Vector4* inVertex, Vector4* outVertex )
{
  multiply( outVertex, &modelViewProjection, *inVertex );
}

void render_draw_pixel( int x, int y, Color& color)
{
  s_cpu_renderer.aFrameBuffer[y*s_renderer.iWidth+x] = color;
}

void render_draw_triangle(const Matrix& modelViewProjection, int tri, uint16* indicies, Vector4* verts  )
{
  Vector4 v[3];

  // transform verts
  render_apply_vertex_shader( modelViewProjection, tri + 0, verts + indicies[ tri + 0 ], &v[0]);
  render_apply_vertex_shader( modelViewProjection, tri + 1, verts + indicies[ tri + 1 ], &v[1]);
  render_apply_vertex_shader( modelViewProjection, tri + 2, verts + indicies[ tri + 2 ], &v[2]);

  // find triangle bounding box
  int maxX = math_max(v[0].x, math_max(v[1].x, v[2].x));
  int minX = math_min(v[0].x, math_min(v[1].x, v[2].x));
  int maxY = math_max(v[0].y, math_max(v[1].y, v[2].y));
  int minY = math_min(v[0].y, math_min(v[1].y, v[2].y));

  Vector4 vs1, vs2, q, t1, t2;
  vector4_set( &vs1, v[1].x - v[0].x, v[1].y - v[0].y, 0.0f, 0.0f);
  vector4_set( &vs2, v[2].x - v[0].x, v[2].y - v[0].y, 0.0f, 0.0f);

  Color c;
  c.r = 255;
  c.g = 0;
  c.b = 0;

  for (int x = minX; x <= maxX; x++)
  {
    for (int y = minY; y <= maxY; y++)
    {
      vector4_set( &q, x - v[0].x, y - v[0].y, 0.0f , 0.0f); 

      math_cross( &t1, &q, &vs2 );
      math_cross( &t2, &vs1, &vs2 );
      float s = dot( &t1, &t2 );

      math_cross( &t1, &vs1, &q );
      float t = dot( &t1, &t2 );

      if ( (s >= 0) && (t >= 0) && (s + t <= 1))
        render_draw_pixel(x, y, c);
    }
  }
}

void render_draw_mesh(const Matrix& modelViewProjection, int mesh )
{
  uint16* meshIndicies = s_cpu_renderer.aMeshBlock.uIndexBuffer[mesh];
  int  indexCount = s_cpu_renderer.aMeshBlock.uNumIndices[mesh];

  Vector4* meshVerts = s_cpu_renderer.aMeshBlock.uVertexBuffer[mesh];
  int  vertCount = s_cpu_renderer.aMeshBlock.uNumVerts[mesh];
  int triCount = indexCount / 3;

  for( int t=0; t < triCount; ++t)
  {
    render_draw_triangle( modelViewProjection, t, meshIndicies, meshVerts );
  }
}
