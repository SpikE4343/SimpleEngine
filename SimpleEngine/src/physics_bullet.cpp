/*************************************************************************************
Phyzaks
Copyright (c) 2012 John Rohrssen
Email: johnrohrssen@gmail.com
*************************************************************************************/

#include "config.h"
#include "util.h"
#include "math_game.h"
#include "render.h"
#include "system.h"
#include "physics.h"
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#if USE_BULLET_PHYSICS

#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/btBulletCollisionCommon.h"

enum CollisionShape
{
  PCS_NONE=0,
  PCS_BOX=1,
  PCS_SPHERE=2,
  PCS_CAPSULE=3,
  PCS_CYLINDER=4,
  PCS_MESH=5
};

PACKED(
struct CollisionShapeBlockBase
{
  int iType;
});

PACKED(
struct BoxCollisionShapeBlock : CollisionShapeBlockBase
{
  float fHalfX;
  float fHalfY;
  float fHalfZ;
} );

PACKED(
struct SphereCollisionShapeBlock : CollisionShapeBlockBase
{
  float fRadius;
} );

PACKED(
struct CapsuleCollisionShapeBlock : CollisionShapeBlockBase
{
  float fRadius;
  float fHeight;
} );

PACKED(
struct CylinderCollisionShapeBlock : CollisionShapeBlockBase
{
  float fHalfX;
  float fHalfY;
  float fHalfZ;
} );

PACKED(
struct RidgidBodyBlock
{
  float fMass;
});

struct	BulletMotionState : public btMotionState
{
  int iObjectId;
  int iOwnerId;

  ///synchronizes world transform from user to physics
  virtual void	getWorldTransform(btTransform& centerOfMassWorldTrans ) const;

  ///synchronizes world transform from physics to user
  ///Bullet only calls the update of worldtransform for active objects
  virtual void	setWorldTransform(const btTransform& centerOfMassWorldTrans);
};


struct PhysicsObjectBlock
{
  enum Size { BLOCK_SIZE=64 };

  int iTail;

  MEMBER( btCollisionShape*, 	pCollisionShape );
  MEMBER( btRigidBody*, 		pRidgidBody 	);
  MEMBER( Matrix, 			mWorldMatrix 	);
  MEMBER( BulletMotionState,  motionState 	);

  MEMBER( int,  iAssetId 	);
};

struct PhysicsStats
{
  uint32 iObjects;
  float fFrameTime;
};

struct PhysicsData
{
  float fTimeMult;
  int iObjects;

  PhysicsObjectBlock aPhysicsBlock;
};

struct BulletPhysicsData : public PhysicsData
{
  btBroadphaseInterface*	         broadphase;
  btCollisionDispatcher*			 dispatcher;
  btConstraintSolver*				 solver;
  btDefaultCollisionConfiguration* collisionConfiguration;
  btDynamicsWorld*				 dynamicsWorld;
};

static BulletPhysicsData s_physics;

int physics_initialize()
{
  PROFILE_FUNC();
  s_physics.fTimeMult = 1.0f;
  s_physics.iObjects = 0;

  s_physics.collisionConfiguration = new btDefaultCollisionConfiguration();

  s_physics.dispatcher = new btCollisionDispatcher(s_physics.collisionConfiguration);
  s_physics.broadphase = new btDbvtBroadphase();

  btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
  s_physics.solver = sol;
  s_physics.dynamicsWorld = new btDiscreteDynamicsWorld( s_physics.dispatcher,
    s_physics.broadphase,
    s_physics.solver,
    s_physics.collisionConfiguration);

  s_physics.dynamicsWorld->setGravity(btVector3(0, -10.0f,0));

  s_physics.dynamicsWorld->setWorldUserInfo( (void*)&s_physics );

  return 1;
}

int physics_update(float fDt)
{
  PROFILE_FUNC();
  if (s_physics.dynamicsWorld)
  {
    s_physics.dynamicsWorld->stepSimulation(fDt * s_physics.fTimeMult, 0, 0.05f*s_physics.fTimeMult);
    //optional but useful: debug drawing
    //s_physics.dynamicsWorld->debugDrawWorld();
  }

  return 1;
}

int physics_shutdown()
{
  delete s_physics.dynamicsWorld;
  delete s_physics.solver;
  delete s_physics.broadphase;
  delete s_physics.dispatcher;
  delete s_physics.collisionConfiguration;

  return 1;
}

void physics_set_timemult( float fTimeMult)
{
  s_physics.fTimeMult = fTimeMult;
}

void physics_set_gravity( const Vector4& vGravity)
{
  btVector3 vbtGrav(vGravity.x, vGravity.y, vGravity.z );
  s_physics.dynamicsWorld->setGravity(vbtGrav);
}

int physics_create_object(int ownerId, int assetid)
{
  int object = ++(s_physics.aPhysicsBlock.iTail);

  s_physics.aPhysicsBlock.mWorldMatrix[object] = matrix_identity();
  s_physics.aPhysicsBlock.pCollisionShape[object] = NULL;
  s_physics.aPhysicsBlock.pRidgidBody[object] = NULL;
  s_physics.aPhysicsBlock.iAssetId[object] = assetid;

  s_physics.aPhysicsBlock.motionState[object].iObjectId = object;
  s_physics.aPhysicsBlock.motionState[object].iOwnerId  = ownerId;

  assetmanager_load(assetid);


  return object;
}

int physics_object_loaded( int iObj )
{
  PROFILE_FUNC();
  int assetid = s_physics.aPhysicsBlock.iAssetId[iObj];
  if( assetid == -1 )
  {
    // no asset for this object to render
    return 0;
  }

  int ready = 1;

  AssetLoadState load_state = asset_load_state(assetid);

  switch( load_state )
  {
  case ALS_READY:
  case ALS_LOADED:
    {
      // asset for this object has been loaded into memory
      // and needs to be processed by this renderer
      if( s_physics.aPhysicsBlock.pRidgidBody[iObj] == NULL && 
        s_physics.aPhysicsBlock.pCollisionShape[iObj] == NULL )
      {
        // make sure all dependent assets are loaded
        Asset* pAsset = assetmanager_get_data( assetid );
        if( pAsset == NULL )
          break;

        if( pAsset->pMainBlock->u32FourCC != AT_PHYSICS )
          break;

        for(int block =0; block < pAsset->u32NumDataBlocks; ++block )
        {
          AssetBlock* pBlock = pAsset->pDataBlock[block];

          switch( pBlock->u32FourCC )
          {
          case AT_PHYSICS_COLLISION:
            {
              CollisionShapeBlockBase* pColBase = (CollisionShapeBlockBase*)asset_block_get_data((AssetBlock*)pBlock);

              switch( pColBase->iType )
              {
              case PCS_BOX:
                {
                  BoxCollisionShapeBlock* pShape = (BoxCollisionShapeBlock*)pColBase;
                  s_physics.aPhysicsBlock.pCollisionShape[iObj] = new btBoxShape(btVector3(pShape->fHalfX, pShape->fHalfY, pShape->fHalfZ));
                }

              case PCS_SPHERE:
                {
                  SphereCollisionShapeBlock* pShape = (SphereCollisionShapeBlock*)pColBase;
                  s_physics.aPhysicsBlock.pCollisionShape[iObj] = new  btSphereShape( 1.0f);
                }
                break;

              case PCS_CAPSULE:
                {
                  CapsuleCollisionShapeBlock* pShape = (CapsuleCollisionShapeBlock*)pColBase;
                  s_physics.aPhysicsBlock.pCollisionShape[iObj] = new btCapsuleShape(pShape->fRadius, pShape->fHeight);
                }
                break;

              case PCS_CYLINDER:
                {
                  CylinderCollisionShapeBlock* pShape = (CylinderCollisionShapeBlock*)pColBase;
                  s_physics.aPhysicsBlock.pCollisionShape[iObj] = new btCylinderShape(btVector3(pShape->fHalfX, pShape->fHalfY, pShape->fHalfZ));
                }
                break;
              }
            }
            break;

          case AT_PHYSICS_DYNAMICS:
            {
              if( s_physics.aPhysicsBlock.pCollisionShape[iObj] != NULL )
              {
                RidgidBodyBlock* pRidgidBody = (RidgidBodyBlock*)asset_block_get_data((AssetBlock*)pBlock);

                btVector3 localInertia(0,0,0);
                s_physics.aPhysicsBlock.pCollisionShape[iObj]->calculateLocalInertia(pRidgidBody->fMass,localInertia);

                //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
                btRigidBody::btRigidBodyConstructionInfo rbInfo(pRidgidBody->fMass, &s_physics.aPhysicsBlock.motionState[iObj],
                  s_physics.aPhysicsBlock.pCollisionShape[iObj],
                  localInertia);

                s_physics.aPhysicsBlock.pRidgidBody[iObj] = new btRigidBody(rbInfo);
              }
              else
              {
                ready = 0;
              }

            }
            break;
          }
        }

        asset_set_load_state( assetid, ALS_READY );

        if( s_physics.aPhysicsBlock.pRidgidBody[iObj] != NULL )
        {
          s_physics.dynamicsWorld->addRigidBody(s_physics.aPhysicsBlock.pRidgidBody[iObj]);
        }

        ready = 0;
      }


    }
    break;

  case ALS_UNLOAD:
  case ALS_UNLOAD_RELOAD:
    {

      ready = 0;
    }
    break;

  default:
    ready = 0;
    break;
  }

  return ready;
}

int physics_object_get_transform( int id, Matrix* pMat)
{
  *pMat = s_physics.aPhysicsBlock.mWorldMatrix[id];
  return 1;
}

int physics_object_set_transform( int id, Matrix* pMat)
{
  PROFILE_FUNC();
  s_physics.aPhysicsBlock.mWorldMatrix[id] = *pMat;

  if( s_physics.aPhysicsBlock.pRidgidBody[id] != NULL )
  {
    btTransform worldTrans = s_physics.aPhysicsBlock.pRidgidBody[id] -> getCenterOfMassTransform();
    Matrix& world = s_physics.aPhysicsBlock.mWorldMatrix[id];

    worldTrans.getOrigin().setValue(world.m[3].x, world.m[3].y, world.m[3].z);

    btMatrix3x3& basis = worldTrans.getBasis();
    basis[0].setValue(world.m[0].x, world.m[0].y, world.m[0].z);
    basis[1].setValue(world.m[1].x, world.m[1].y, world.m[1].z);
    basis[2].setValue(world.m[2].x, world.m[2].y, world.m[2].z);
    s_physics.aPhysicsBlock.pRidgidBody[id]->activate();
    s_physics.aPhysicsBlock.pRidgidBody[id]->setCenterOfMassTransform(worldTrans);
  }
  return 1;
}

void BulletMotionState::getWorldTransform(btTransform& centerOfMassWorldTrans ) const
{
  PROFILE_FUNC();
  Matrix& world = s_physics.aPhysicsBlock.mWorldMatrix[iObjectId];

  centerOfMassWorldTrans.getOrigin().setValue(world.m[3].x, world.m[3].y, world.m[3].z);

  btMatrix3x3& basis = centerOfMassWorldTrans.getBasis();
  basis[0].setValue(world.m[0].x, world.m[0].y, world.m[0].z);
  basis[1].setValue(world.m[1].x, world.m[1].y, world.m[1].z);
  basis[2].setValue(world.m[2].x, world.m[2].y, world.m[2].z);

  //system_log( "motionstate::getWorldTransform %d, owner: %d",  iObjectId, iOwnerId);
}


void BulletMotionState::setWorldTransform(const btTransform& centerOfMassWorldTrans)
{
  PROFILE_FUNC();
  Matrix& world = s_physics.aPhysicsBlock.mWorldMatrix[iObjectId];
  const btMatrix3x3& basis = centerOfMassWorldTrans.getBasis();


  world.m[0].x = basis[0].x();
  world.m[0].y = basis[0].y();
  world.m[0].z = basis[0].z();

  world.m[1].x = basis[1].x();
  world.m[1].y = basis[1].y();
  world.m[1].z = basis[1].z();

  world.m[2].x = basis[2].x();
  world.m[2].y = basis[2].y();
  world.m[2].z = basis[2].z();


  matrix_set_translation( &world, centerOfMassWorldTrans.getOrigin().x(),
    centerOfMassWorldTrans.getOrigin().y(),
    centerOfMassWorldTrans.getOrigin().z() );

  game_object_update_transform( iOwnerId, &world);

  //system_log( "motionstate::setWorldTransform %d, owner: %d",  iObjectId, iOwnerId);

}

#endif
