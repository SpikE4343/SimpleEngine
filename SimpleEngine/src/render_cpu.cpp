#include "config.h"
#include "block.h"
#include "math_game.h"
#include "render.h"

//#if USE_ARDUINO_RENDERER

#include <TVout.h>
#include <video_gen.h>
#include <font6x8.h>

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

  MEMBER( uint, uVertexBuffer);
  MEMBER( uint, uNumVerts );

  //MEMBER( uint, uVertexNormalBuffer);
  //MEMBER( uint, uNumVertNormals );

  MEMBER( uint, uIndexBuffer);
  MEMBER( uint, uNumIndices );

  MEMBER( uint, uTexCoordBuffer);
  MEMBER( uint, uNumTexCoords );
};

struct TextureBlock
{
  int iTail;
  MEMBER( uint, uTexture);
  MEMBER( uint, uWidth);
  MEMBER( uint, uHeight);
};


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
  if( aRenderBlock.iTail - 1 >= OBJECT_BLOCK_SIZE )
    return -1;

  int index = aRenderBlock.iTail++;

  aRenderBlock.uFlags[index] = 1;
  aRenderBlock.uVisible[index] = 0;
  aRenderBlock.fRadius[index] = 1.0f;

  aRenderBlock.aWorldMatrix[index] = Matrix::IDENTITY;

  return index;
}

int render_clone_renderobject(int existing, Vector4 pos)
{
  // to many objects in the block
  if( aRenderBlock.iTail - 1 >= OBJECT_BLOCK_SIZE )
    return -1;

  int index = aRenderBlock.iTail++;

  RENDER_OBJ( index, uFlags )   = 1;
  RENDER_OBJ( index, uVisible ) = 0;
  RENDER_OBJ( index, fRadius )  = 1.0f;

  RENDER_OBJ( index, aWorldMatrix ) = matrix_identity();

  RENDER_OBJ( index, uShader ) = RENDER_OBJ( existing, uShader );
  RENDER_OBJ( index, uMesh )   = RENDER_OBJ( existing, uMesh );

  RENDER_OBJ( index, aWorldMatrix ).m[3] = pos;
  return index;
}

//RendererData& render_data()
//{
//  return s_renderer;
//}

// ======================================
int render_create_shader()
{
  // to many objects in the block
  if( aShaderBlock.iTail < 0 || 
    aShaderBlock.iTail - 1 >= OBJECT_BLOCK_SIZE )
    return -1;

  int index = aShaderBlock.iTail++;
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
  return &viewMatrix;
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

  multiply( &viewProjection, &viewMatrix, &projMatrix);

  add( &viewFrustum.left, &viewProjection.m[3], &viewProjection.m[0] );
  sub( &viewFrustum.right, &viewProjection.m[3], &viewProjection.m[0] );

  sub( &viewFrustum.top, &viewProjection.m[3], &viewProjection.m[1] );
  add( &viewFrustum.bottom, &viewProjection.m[3], &viewProjection.m[1] );

  viewFrustum.front = viewProjection.m[2];

  sub( &viewFrustum.back, &viewProjection.m[3], &viewProjection.m[2] );

  plane_normalize( &viewFrustum.top );
  plane_normalize( &viewFrustum.bottom );

  plane_normalize( &viewFrustum.left );
  plane_normalize( &viewFrustum.right );

  plane_normalize( &viewFrustum.front );
  plane_normalize( &viewFrustum.back );

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
  for( int i=0; i < aRenderBlock.iTail; ++i )
  {
    float fRadius = aRenderBlock.fRadius[i];

    // 0x80000000
    pos = aRenderBlock.aWorldMatrix[i].m[3];

    fDot = dot( &viewFrustum.front, &pos );
    fDot += fRadius + viewFrustum.front.w;
    res = FLOAT_GET_SIGN( fDot );

    fDot = dot( &viewFrustum.back, &pos );
    fDot += fRadius + viewFrustum.back.w;
    res |= FLOAT_GET_SIGN( fDot );

    fDot = dot( &viewFrustum.top, &pos );
    fDot += fRadius + viewFrustum.top.w;
    res |= FLOAT_GET_SIGN( fDot );

    fDot = dot( &viewFrustum.bottom, &pos );
    fDot += fRadius + viewFrustum.bottom.w;
    res |= FLOAT_GET_SIGN( fDot );

    fDot = dot( &viewFrustum.left, &pos );
    fDot += fRadius + viewFrustum.left.w;
    res |= FLOAT_GET_SIGN( fDot );

    fDot = dot( &viewFrustum.right, &pos );
    fDot += fRadius + viewFrustum.right.w;
    res |= FLOAT_GET_SIGN( fDot );

    aRenderBlock.uVisible[i] = res;
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
  if( !aRenderBlock.uVisible[iObj] )
    return 0;

  //  if( iCurrentMesh != aRenderBlock.uMesh[iObj])
  //  {
  //    PROFILE_BLOCK( obj_set_buffers );
  //    iCurrentMesh = aRenderBlock.uMesh[iObj];
  //
  //    // set vertex buffer
  //    glBindBuffer(GL_ARRAY_BUFFER, aMeshBlock.uVertexBuffer[iCurrentMesh]);
  //    checkGlError("glBindBuffer");
  //
  //    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
  //    checkGlError("glVertexAttribPointer");
  //
  //    glEnableVertexAttribArray(0);
  //    checkGlError("glEnableVertexAttribArray");
  //
  //    glBindBuffer(GL_ARRAY_BUFFER, aMeshBlock.uVertexNormalBuffer[iCurrentMesh]);
  //    glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
  //    glEnableVertexAttribArray(1);
  //
  //
  //    // set texture coordinate buffer
  //    glBindBuffer(GL_ARRAY_BUFFER, aMeshBlock.uTexCoordBuffer[iCurrentMesh]);
  //    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, 0 );
  //    glEnableVertexAttribArray( 2 );
  //
  //   
  //    // bind indices
  //    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aMeshBlock.uIndexBuffer[iCurrentMesh]);
  //    checkGlError("glBindBuffer");
  //  }
  //
  //  uint iShaderIndex = aRenderBlock.uShader[iObj];
  //  if( iCurrentShader != iShaderIndex )
  //  {
  //    iCurrentShader = iShaderIndex;
  //
  //    PROFILE_BLOCK( obj_set_shader_params );
  //
  //    //system_log( "draw program: %u", aShaderBlock.uProgram[iCurrentShader]);
  //
  //    uint program = aShaderBlock.uProgram[iCurrentShader];
  //    glUseProgram ( program );
  //    checkGlError("glUseProgram");
  //  }
  //
  //  Matrix mvp = matrix_identity();
  //  multiply( &mvp, &aRenderBlock.aWorldMatrix[iObj], &viewMatrix );
  //
  //  Matrix tempNormalMat = matrix_identity();
  //  matrix_inverse_nonui(&tempNormalMat, &mvp );
  //  transpose( &tempNormalMat );
  //
  //  multiply( &mvp, &projMatrix );
  //
  //
  //
  //
  //  glUniformMatrix4fv( aShaderBlock.uWorldMatrixParam[iShaderIndex],     1, GL_FALSE, &aRenderBlock.aWorldMatrix[iObj].m[0].x);
  //  glUniformMatrix4fv( aShaderBlock.uViewMatrixParam[iShaderIndex],      1, GL_FALSE, &viewMatrix.m[0].x);
  //  //glUniformMatrix4fv( aShaderBlock.uProjMatrixParam[iShaderIndex],      1, GL_FALSE, &projMatrix.m[0].x);
  //  glUniformMatrix4fv( aShaderBlock.uNormalMatrixParam[iShaderIndex],    1, GL_FALSE, &tempNormalMat.m[0].x);
  //  glUniformMatrix4fv( aShaderBlock.uFinalTransformParam[iShaderIndex],  1, GL_FALSE, &mvp.m[0].x);
  //  checkGlError("glUniformMatrix4fv");
  //  //glUniformMatrix4fv( aShaderBlock.uFinalTransformParam[iShaderIndex],  1, GL_FALSE, &Matrix::IDENTITY.m[0].x);
  //
  //  glUniform4fv (aShaderBlock.uLightPosParam[iShaderIndex], 1, (GLfloat*)&vSunPos.x);
  //  glUniform4fv (aShaderBlock.uLightColorParam[iShaderIndex], 1, (GLfloat*)&vSunColor.x);
  //    
  //
  //  if( aShaderBlock.uTexture0[iShaderIndex] )
  //  {
  //    // Bind the texture
  //    int texture = aShaderBlock.uTexture0[iShaderIndex];
  //    glActiveTexture ( GL_TEXTURE0 );
  //    glBindTexture ( GL_TEXTURE_2D, aTextureBlock.uTexture[texture] );
  //        
  //    // Set the sampler texture unit to 0
  //    glUniform1i ( aShaderBlock.uTexture0Loc[iShaderIndex], 0 );
  //  }
  //
  //  // draw it already
  //  //system_log( "drawing triangles: %u", aMeshBlock.uNumIndices[iCurrentMesh]);
  //  glDrawElements(GL_TRIANGLES, aMeshBlock.uNumIndices[iCurrentMesh], GL_UNSIGNED_SHORT, (void*)0);
  //  checkGlError("glDrawElements");
  //
  //  //system_log( "drawing %d", iObj);

  return 1;
}

TVout tv;
int render_initialize(int width, int height)
{
  iWidth = width;
  iHeight = height;


  fFarPlane = 20000.0f;
  fNearPlane = 1.0f;
  fFOV = 55.0f;

  //vector4_set( &vSunPos, 0.0f, 100000.0f, 1000000.0f, 1.0f );
  //vector4_set( &vSunColor, 1.0f, 0.0f, 0.0f, 1.0f );

  //viewDebugMatrix = matrix_identity();

  //viewMatrix = Matrix::IDENTITY;
  //vector4_set( &viewMatrix.m[3], 0.0f, 0.0f, -20.0f, 1.0f );

  //  aRenderBlock.iTail  = 0;
  //  aMeshBlock.iTail    = 0;
  //  aShaderBlock.iTail  = 0;
  //  aTextureBlock.iTail = 0;

  fScreenRatio = iWidth / (float)iHeight;



  matrix_create_perspective( &projMatrix, fFOV , fScreenRatio, fNearPlane, fFarPlane);
  render_build_frustum();


  tv.begin(NTSC, iWidth, iHeight);
  tv.select_font(font6x8);
  tv.clear_screen();
  return 1;
}

int render_begin_frame()
{
  //Serial.println("begin_frame");

  // TODO: cache until fov changes
  //  projMatrix = matrix_identity();
  //  matrix_create_perspective( &projMatrix, fFOV, fScreenRatio, fNearPlane, fFarPlane);
  //  render_build_frustum();

  tv.delay_frame(1);
  tv.clear_screen();

  return 1;
}

int render_draw_text( const char* pText, int x, int y )
{
  tv.select_font(font6x8);
  tv.print( x, y, pText );
  return 1;
}

int render_end_frame()
{
  PROFILE_FUNC();

  ++iFrameCount;
  //eglSwapBuffers ( eglDisplay, eglSurface );
  //checkGlError("eglSwapBuffers");
  //tv.delay(1000);
  return 1;
}

int render_shutdown()
{
  return 1;
}
