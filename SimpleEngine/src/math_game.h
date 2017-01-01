#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#define PI 3.1415926535897932384626433832795f
#define DEG2RAD PI / 360.0f


#define FLOAT_SIGN_MASK 0x80000000
#define FLOAT_GET_SIGN( f ) (*((int*)&f)) & FLOAT_SIGN_MASK

struct Vector4
{
  float x;
  float y;
  float z;
  float w;

  static Vector4 Zero;
  static Vector4 Up;
  static Vector4 Right;
  static Vector4 Forward;
};

struct Frustum
{
  Vector4 front;
  Vector4 back;
  Vector4 top;
  Vector4 bottom;
  Vector4 left;
  Vector4 right;
};

inline void vector4_set( Vector4* pDest, float x, float y, float z, float w )
{
  pDest->x = x;
  pDest->y = y;
  pDest->z = z;
  pDest->w = w;
}

typedef Vector4 mat44[4];
struct Matrix
{
  mat44   m;

  static Matrix IDENTITY;
};

//
// \brief multiply matrix specified by result with a perspective matrix and return new matrix in result
/// \param result Specifies the input matrix.  new matrix is returned in result.
/// \param left, right Coordinates for the left and right vertical clipping planes
/// \param bottom, top Coordinates for the bottom and top horizontal clipping planes
/// \param nearZ, farZ Distances to the near and far depth clipping planes.  Both distances must be positive.
//
void matrix_create_frustum(Matrix *result, float left, float right, float bottom, float top, float nearZ, float farZ);

//
/// \brief multiply matrix specified by result with a perspective matrix and return new matrix in result
/// \param result Specifies the input matrix.  new matrix is returned in result.
/// \param fovy Field of view y angle in degrees
/// \param aspect Aspect ratio of screen
/// \param nearZ Near plane distance
/// \param farZ Far plane distance
//
void matrix_create_perspective(Matrix *result, float fovy, float aspect, float nearZ, float farZ);

//
/// \brief multiply matrix specified by result with a perspective matrix and return new matrix in result
/// \param result Specifies the input matrix.  new matrix is returned in result.
/// \param left, right Coordinates for the left and right vertical clipping planes
/// \param bottom, top Coordinates for the bottom and top horizontal clipping planes
/// \param nearZ, farZ Distances to the near and far depth clipping planes.  These values are negative if plane is behind the viewer
//
void matrix_create_ortho(Matrix *result, float left, float right, float bottom, float top, float nearZ, float farZ);

//
/// \brief perform the following operation - result matrix = srcA matrix * srcB matrix
/// \param result Returns multiplied matrix
/// \param srcA, srcB Input matrices to be multiplied
//
void multiply(Matrix *result, Matrix *srcA, Matrix *srcB);
void multiply(Matrix *srcA, Matrix *srcB);
void multiply(Vector4 *r, const Matrix *m, Vector4 v);
void multiply33(Vector4 *r, const Matrix *m, Vector4 v);

void scale( Vector4* pResult, const Vector4* pVector, float scalar );
void div( Vector4* pResult, const Vector4* pVector, float scalar );

void add( Vector4* pResult, const Vector4* pVector, float scalar );
void sub( Vector4* pResult, const Vector4* pVector, float scalar );

void add( Vector4* pResult, const Vector4* pVector, const Vector4* pRhs );
void sub( Vector4* pResult, const Vector4* pVector, const Vector4* pRhs );

float dot( const Vector4* pVector, const Vector4* pRhs );
float dot( const Vector4* pVector, const Vector4* pRhs, float scalar );
void math_cross(Vector4* result, const Vector4* v1, const Vector4* v2 );

float vector4_length( const Vector4* pVector );
void vector4_normalize( Vector4* v );

void plane_normalize( Vector4* vResult, Vector4* vPlane );

void plane_normalize( Vector4* vPlane );

bool matrix_equal(const Matrix *lhs, const Matrix *rhs);

void matrix_rotation_x( Matrix* result, float fRadianAngle );
void matrix_rotation_y( Matrix* result, float fRadianAngle );
void matrix_rotation_z( Matrix* result, float fRadianAngle );

void matrix_inverse_nonui( Matrix* rmat, const Matrix* mat );

void transpose( Matrix* rmat, const Matrix* mat );
void transpose( Matrix* mat );


void rotate(Matrix *result, float angle, const Vector4* axis);
void rotate(Matrix *result, float angle, float x, float y, float z);

void scale(Matrix *result, float sx, float sy, float sz);
void translate(Matrix *result, float tx, float ty, float tz);
void translate(Matrix *result, const Vector4& vPos);

float math_min ( float a, float b );
float math_max ( float a, float b );

static Matrix& matrix_identity()
{ 
  return Matrix::IDENTITY; 
}

#endif
