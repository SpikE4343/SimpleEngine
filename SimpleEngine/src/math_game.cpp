
#include <string.h>
#include <math.h>
#include "math_game.h"

Vector4 Vector4::Zero = { 0.0f, 0.0f, 0.0f, 1.0f };

Matrix Matrix::IDENTITY = 
{ 
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

void scale(Matrix *result, float sx, float sy, float sz)
{
    result->m[0].x *= sx;
    result->m[0].y *= sx;
    result->m[0].z *= sx;
    result->m[0].w *= sx;

    result->m[1].x *= sy;
    result->m[1].y *= sy;
    result->m[1].z *= sy;
    result->m[1].w *= sy;

    result->m[2].x *= sz;
    result->m[2].y *= sz;
    result->m[2].z *= sz;
    result->m[2].w *= sz;
}

void translate(Matrix *result, float tx, float ty, float tz)
{
	//Vector4 v = { tx, ty, tz, 0.0f };
	//multiply33( &v, result, v );
	//add( &result->m[3], &result->m[3], &v);
	
	result->m[3].x += (result->m[0].x * tx + result->m[1].x * ty + result->m[2].x * tz);
    result->m[3].y += (result->m[0].y * tx + result->m[1].y * ty + result->m[2].y * tz);
    result->m[3].z += (result->m[0].z * tx + result->m[1].z * ty + result->m[2].z * tz);
    //result->m[3].w += (result->m[0].w * tx + result->m[1].w * ty + result->m[2].w * tz);
}

void rotate(Matrix *result, float angle, const Vector4* axis)
{
    rotate( result, angle, axis->x, axis->y, axis->z );
}

void rotate(Matrix *result, float angle, float x, float y, float z)
{
	float sinAngle, cosAngle;
	float mag = sqrtf(x * x + y * y + z * z);
	angle *= (PI / 180.0f); 
	
	sinAngle = sinf ( angle );
	cosAngle = cosf ( angle );
   
	float xx, yy, zz, xy, yz, zx, xs, ys, zs;
	float oneMinusCos;
	Matrix rotMat;
   
	x /= mag;
	y /= mag;
	z /= mag;

	xx = x * x;
	yy = y * y;
	zz = z * z;
	xy = x * y;
	yz = y * z;
	zx = z * x;
	xs = x * sinAngle;
	ys = y * sinAngle;
	zs = z * sinAngle;
	oneMinusCos = 1.0f - cosAngle;

	rotMat.m[0].x = (oneMinusCos * xx) + cosAngle;
	rotMat.m[0].y = (oneMinusCos * xy) - zs;
	rotMat.m[0].z = (oneMinusCos * zx) + ys;
	rotMat.m[0].w = 0.0F; 

	rotMat.m[1].x = (oneMinusCos * xy) + zs;
	rotMat.m[1].y = (oneMinusCos * yy) + cosAngle;
	rotMat.m[1].z = (oneMinusCos * yz) - xs;
	rotMat.m[1].w = 0.0F;

	rotMat.m[2].x = (oneMinusCos * zx) - ys;
	rotMat.m[2].y = (oneMinusCos * yz) + xs;
	rotMat.m[2].z = (oneMinusCos * zz) + cosAngle;
	rotMat.m[2].w = 0.0F; 

	rotMat.m[3].x = 0.0F;
	rotMat.m[3].y = 0.0F;
	rotMat.m[3].z = 0.0F;
	rotMat.m[3].w = 1.0F;

	multiply( result, &rotMat, result );
}

void matrix_create_perspective(Matrix *result, float fovy, float aspect, float nearZ, float farZ)
{
	float frustumW, frustumH;
   
    frustumH = tanf( (fovy / 360.0f * PI) *0.5f ) * nearZ;
    frustumW = frustumH * aspect;

    float left   = -frustumW;
	float right  =  frustumW;
	float bottom = -frustumH;
	float top    =  frustumH;

	float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;

    if ( (nearZ <= 0.0f) || (farZ <= 0.0f) || (deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f) )
         return;

    vector4_set(&result->m[0], 2.0f * nearZ / deltaX,    0.0f,                      (right + left) / deltaX,    0.0f );
	vector4_set(&result->m[1], 0.0f,                     2.0f * nearZ / deltaY,     (top + bottom) / deltaY,    0.0f ); 
	vector4_set(&result->m[2], 0.0f,					 0.0f,                      -(nearZ + farZ) / deltaZ,   -2.0f * nearZ * farZ / deltaZ );
	vector4_set(&result->m[3], 0.0f,                     0.0f,                      -1.0f,                      0.0f );

	//transpose( result );

}

void matrix_create_ortho(Matrix *result, float left, float right, float bottom, float top, float nearZ, float farZ)
{
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;
    Matrix    ortho;

    if ( (deltaX == 0.0f) || (deltaY == 0.0f) || (deltaZ == 0.0f) )
        return;

    ortho = Matrix::IDENTITY;
    ortho.m[0].x = 2.0f / deltaX;
    ortho.m[3].x = -(right + left) / deltaX;
    ortho.m[1].y = 2.0f / deltaY;
    ortho.m[3].y = -(top + bottom) / deltaY;
    ortho.m[2].z = -2.0f / deltaZ;
    ortho.m[3].z = -(nearZ + farZ) / deltaZ;

    multiply(result, &ortho, result);
}

void matrix_rotation_x( Matrix* result, float fRadianAngle )
{
	float fCos = cosf(fRadianAngle);
	float fSin = sinf(fRadianAngle);
	
	vector4_set( &result->m[0], 1.0f, 0.0f, 0.0f,  0.0f );
	vector4_set( &result->m[1], 0.0f, fCos, -fSin, 0.0f );
	vector4_set( &result->m[2], 0.0f, fSin, fCos,  0.0f );
	vector4_set( &result->m[3], 0.0f, 0.0f, 0.0f,  1.0f );
}

void matrix_rotation_y( Matrix* result, float fRadianAngle )
{
	float fCos = cosf(fRadianAngle);
	float fSin = sinf(fRadianAngle);
	 
	vector4_set( &result->m[0], fCos,  0.0f, fSin, 0.0f );
	vector4_set( &result->m[1], 0.0f,  1.0f, 0.0f, 0.0f );
	vector4_set( &result->m[2], -fSin, 0.0f, fCos, 0.0f );
	vector4_set( &result->m[3], 0.0f,  0.0f, 0.0f, 0.0f );
}

void matrix_rotation_z( Matrix* result, float fRadianAngle )
{
	float fCos = cosf(fRadianAngle);
	float fSin = sinf(fRadianAngle);
	 
	vector4_set( &result->m[0], fCos, -fSin, 0.0f, 0.0f );
	vector4_set( &result->m[1], fSin, fCos,  0.0f, 0.0f );
	vector4_set( &result->m[2], 0.0f, 0.0f,  1.0f, 0.0f );
	vector4_set( &result->m[3], 0.0f, 0.0f,  0.0f, 1.0f );
}


void multiply(Matrix *result, Matrix *srcA, Matrix *srcB)
{
    Matrix    tmp;
    int         i;

	for (i=0; i<4; i++)
	{
		tmp.m[i].x =	(srcA->m[i].x * srcB->m[0].x) +
						(srcA->m[i].y * srcB->m[1].x) +
						(srcA->m[i].z * srcB->m[2].x) +
						(srcA->m[i].w * srcB->m[3].x) ;

		tmp.m[i].y =	(srcA->m[i].x * srcB->m[0].y) + 
						(srcA->m[i].y * srcB->m[1].y) +
						(srcA->m[i].z * srcB->m[2].y) +
						(srcA->m[i].w * srcB->m[3].y) ;

		tmp.m[i].z =	(srcA->m[i].x * srcB->m[0].z) + 
						(srcA->m[i].y * srcB->m[1].z) +
						(srcA->m[i].z * srcB->m[2].z) +
						(srcA->m[i].w * srcB->m[3].z) ;

		tmp.m[i].w =	(srcA->m[i].x * srcB->m[0].w) + 
						(srcA->m[i].y * srcB->m[1].w) +
						(srcA->m[i].z * srcB->m[2].w) +
						(srcA->m[i].w * srcB->m[3].w) ;
	}
    
	memcpy(result, &tmp, sizeof(Matrix));
}

void multiply(Matrix *srcA, Matrix *srcB)
{
    Matrix    tmp;
    int         i;

	for (i=0; i<4; i++)
	{
		tmp.m[i].x =	(srcA->m[i].x * srcB->m[0].x) +
						(srcA->m[i].y * srcB->m[1].x) +
						(srcA->m[i].z * srcB->m[2].x) +
						(srcA->m[i].w * srcB->m[3].x) ;

		tmp.m[i].y =	(srcA->m[i].x * srcB->m[0].y) + 
						(srcA->m[i].y * srcB->m[1].y) +
						(srcA->m[i].z * srcB->m[2].y) +
						(srcA->m[i].w * srcB->m[3].y) ;

		tmp.m[i].z =	(srcA->m[i].x * srcB->m[0].z) + 
						(srcA->m[i].y * srcB->m[1].z) +
						(srcA->m[i].z * srcB->m[2].z) +
						(srcA->m[i].w * srcB->m[3].z) ;

		tmp.m[i].w =	(srcA->m[i].x * srcB->m[0].w) + 
						(srcA->m[i].y * srcB->m[1].w) +
						(srcA->m[i].z * srcB->m[2].w) +
						(srcA->m[i].w * srcB->m[3].w) ;
	}
    
	memcpy(srcA, &tmp, sizeof(Matrix));
}

void multiply(Vector4 *r, const Matrix *m, Vector4 v)
{
    float fInvW = 1.0f / ( m->m[3].x * v.x + m->m[3].y * v.y + m->m[3].z * v.z + m->m[3].w );

    r->x = ( m->m[0].x * v.x + m->m[0].y * v.y + m->m[0].z * v.z + m->m[0].w ) * fInvW;
    r->y = ( m->m[1].x * v.x + m->m[1].y * v.y + m->m[1].z * v.z + m->m[1].w ) * fInvW;
    r->z = ( m->m[2].x * v.x + m->m[2].y * v.y + m->m[2].z * v.z + m->m[2].w ) * fInvW;
	//r->w = fInvW;
}

void multiply33(Vector4 *r, const Matrix *m, Vector4 v)
{
    r->x = ( m->m[0].x * v.x + m->m[0].y * v.y + m->m[0].z * v.z );
    r->y = ( m->m[1].x * v.x + m->m[1].y * v.y + m->m[1].z * v.z );
    r->z = ( m->m[2].x * v.x + m->m[2].y * v.y + m->m[2].z * v.z );
}

void scale( Vector4* pResult, const Vector4* pVector, float scalar )
{
    pResult->x = pVector->x * scalar;
    pResult->y = pVector->y * scalar;
    pResult->z = pVector->z * scalar;
	pResult->z = pVector->w * scalar;
}

void div( Vector4* pResult, const Vector4* pVector, float scalar )
{
	float inv = 1.0f / scalar;
    pResult->x = pVector->x * inv;
    pResult->y = pVector->y * inv;
    pResult->z = pVector->z * inv;
	pResult->z = pVector->w * inv;
}

void add( Vector4* pResult, const Vector4* pVector, float scalar )
{
    pResult->x = pVector->x + scalar;
    pResult->y = pVector->y + scalar;
    pResult->z = pVector->z + scalar;
	pResult->w = pVector->w + scalar;
}

void sub( Vector4* pResult, const Vector4* pVector, float scalar )
{
    pResult->x = pVector->x - scalar;
    pResult->y = pVector->y - scalar;
    pResult->z = pVector->z - scalar;
	pResult->w = pVector->w - scalar;
}

float dot( const Vector4* pVector, const Vector4* pRhs )
{
    return pVector->x * pRhs->x + 
		   pVector->y * pRhs->y + 
		   pVector->z * pRhs->z;
}

float dot( const Vector4* pVector, const Vector4* pRhs, float scalar)
{
    return pVector->x * scalar * pRhs->x + 
		   pVector->y * scalar * pRhs->y + 
		   pVector->z * scalar * pRhs->z +
		   pVector->w * scalar * pRhs->w;
		   
}

void cross(Vector4* result, const Vector4* v1, const Vector4* v2 )
{
    result->x = v1->y * v2->z - v1->z * v2->y;
    result->y = v1->z * v2->x - v1->x * v2->z;
    result->z = v1->x * v2->y - v1->y * v2->x;
}

float length( const Vector4* pVector )
{
    return sqrtf(dot(pVector, pVector));
}

void normalize( Vector4* v )
{
    scale( v, v, 1.0f / length( v ) );
}

void plane_normalize( Vector4* vResult, Vector4* vPlane )
{
	*vResult = *vPlane;
	normalize( vResult );
	vResult->w *= vResult->x / vPlane->x;
}

void plane_normalize( Vector4* vPlane )
{
	float fOldX = vPlane->x;
	normalize( vPlane );
	vPlane->w *= vPlane->x / fOldX;
}

void add( Vector4* pResult, const Vector4* pVector, const Vector4* pRhs )
{
    pResult->x = pVector->x + pRhs->x;
    pResult->y = pVector->y + pRhs->y;
    pResult->z = pVector->z + pRhs->z;
	pResult->w = pVector->w + pRhs->w;
}

void sub( Vector4* pResult, const Vector4* pVector, const Vector4* pRhs )
{
    pResult->x = pVector->x - pRhs->x;
    pResult->y = pVector->y - pRhs->y;
    pResult->z = pVector->z - pRhs->z;
	pResult->w = pVector->w - pRhs->w;
}

bool equal(const Matrix* lhs, const Matrix* rhs)
{
    return  lhs->m[0].x == rhs->m[0].x &&
            lhs->m[0].y == rhs->m[0].y &&
            lhs->m[0].z == rhs->m[0].z &&
            lhs->m[0].w == rhs->m[0].w &&

            lhs->m[1].x == rhs->m[1].x &&
            lhs->m[1].y == rhs->m[1].y &&
            lhs->m[1].z == rhs->m[1].z &&
            lhs->m[1].w == rhs->m[1].w &&

            lhs->m[2].x == rhs->m[2].x &&
            lhs->m[2].y == rhs->m[2].y &&
            lhs->m[2].z == rhs->m[2].z &&
            lhs->m[2].w == rhs->m[2].w &&

            lhs->m[3].x == rhs->m[3].x &&
            lhs->m[3].y == rhs->m[3].y &&
            lhs->m[3].z == rhs->m[3].z &&
            lhs->m[3].w == rhs->m[3].w;
}

void transpose( Matrix* rmat, const Matrix* mat )
{
    const mat44* m = &mat->m;
    mat44* r = &rmat->m;
    
    (*r)[0].x = (*m)[0].x;
    (*r)[0].y = (*m)[1].x;
    (*r)[0].z = (*m)[2].x;
    (*r)[0].w = (*m)[3].x;

    (*r)[1].x = (*m)[0].y;
    (*r)[1].y = (*m)[1].y;
    (*r)[1].z = (*m)[2].y;
    (*r)[1].w = (*m)[3].y;

    (*r)[2].x = (*m)[0].z;
    (*r)[2].y = (*m)[1].z;
    (*r)[2].z = (*m)[2].z;
    (*r)[2].w = (*m)[3].z;

    (*r)[3].x = (*m)[0].w;
    (*r)[3].y = (*m)[1].w;
    (*r)[3].z = (*m)[2].w;
    (*r)[3].w = (*m)[3].w;
}

void transpose( Matrix* mat )
{
    Matrix old = *mat;
    mat44* r = &mat->m;
    
    (*r)[0].x = old.m[0].x;
    (*r)[0].y = old.m[1].x;
    (*r)[0].z = old.m[2].x;
    (*r)[0].w = old.m[3].x;

    (*r)[1].x = old.m[0].y;
    (*r)[1].y = old.m[1].y;
    (*r)[1].z = old.m[2].y;
    (*r)[1].w = old.m[3].y;

    (*r)[2].x = old.m[0].z;
    (*r)[2].y = old.m[1].z;
    (*r)[2].z = old.m[2].z;
    (*r)[2].w = old.m[3].z;

    (*r)[3].x = old.m[0].w;
    (*r)[3].y = old.m[1].w;
    (*r)[3].z = old.m[2].w;
    (*r)[3].w = old.m[3].w;
}

void matrix_inverse_nonui( Matrix* rmat, const Matrix* mat )
{
    const mat44* m = &mat->m;
    mat44* r = &rmat->m;

    float m00 = (*m)[0].x, m01 = (*m)[0].y, m02 = (*m)[0].z, m03 = (*m)[0].w;
    float m10 = (*m)[1].x, m11 = (*m)[1].y, m12 = (*m)[1].z, m13 = (*m)[1].w;
    float m20 = (*m)[2].x, m21 = (*m)[2].y, m22 = (*m)[2].z, m23 = (*m)[2].w;
    float m30 = (*m)[3].x, m31 = (*m)[3].y, m32 = (*m)[3].z, m33 = (*m)[3].w;

    float v0 = m20 * m31 - m21 * m30;
    float v1 = m20 * m32 - m22 * m30;
    float v2 = m20 * m33 - m23 * m30;
    float v3 = m21 * m32 - m22 * m31;
    float v4 = m21 * m33 - m23 * m31;
    float v5 = m22 * m33 - m23 * m32;

    float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
    float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
    float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
    float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

    float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

    (*r)[0].x = t00 * invDet;
    (*r)[1].x = t10 * invDet;
    (*r)[2].x = t20 * invDet;
    (*r)[3].x = t30 * invDet;

    (*r)[0].y = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    (*r)[1].y = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    (*r)[2].y = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    (*r)[3].y = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m10 * m31 - m11 * m30;
    v1 = m10 * m32 - m12 * m30;
    v2 = m10 * m33 - m13 * m30;
    v3 = m11 * m32 - m12 * m31;
    v4 = m11 * m33 - m13 * m31;
    v5 = m12 * m33 - m13 * m32;

    (*r)[0].z = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    (*r)[1].z = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    (*r)[2].z = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    (*r)[3].z = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m21 * m10 - m20 * m11;
    v1 = m22 * m10 - m20 * m12;
    v2 = m23 * m10 - m20 * m13;
    v3 = m22 * m11 - m21 * m12;
    v4 = m23 * m11 - m21 * m13;
    v5 = m23 * m12 - m22 * m13;

    (*r)[0].w = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    (*r)[1].w = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    (*r)[2].w = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    (*r)[3].w = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;
}

void plane_create( Vector4* plane, const Vector4* kP1, const Vector4* kP2, const Vector4* kP3 )
{
	Vector4 kEdge1;
    Vector4 kEdge2;

    sub( &kEdge1, kP2, kP1 );
    sub( &kEdge2, kP1, kP2 );

    cross(plane, &kEdge1, &kEdge2 );
    normalize(plane );
	
    plane->w = -dot(plane, kP1 );
}

#if 0
float esPlanePointIntersection( const ESPlane* plane, const Vector4* vOrigin )
{
    return vector4_dot( plane->vNormal, vOrigin ) + plane.d;
}

float esPlaneSphereIntersection( const ESPlane& plane, const Vector4& vOrigin, float fRadius )
{
    return esPlanePointIntersection( plane, vOrigin );
}

bool esSphereIntersectsPlane( const ESPlane& plane, const Vector4& vOrigin, float fRadius )
{
    return ( Vector4Dot( plane.vNormal, vOrigin ) + plane.d ) <= fRadius;
}
*/

#endif
