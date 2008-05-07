/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


/**
 * @mainpage Bullet Documentation
 *
 * @section intro_sec Introduction
 * Bullet Collision Detection & Physics SDK
 *
 * Bullet is a Collision Detection and Rigid Body Dynamics Library. The Library is Open Source and free for commercial use, under the ZLib license ( http://opensource.org/licenses/zlib-license.php ).
 *
 * There is the Physics Forum for Feedback and bteral Collision Detection and Physics discussions.
 * Please visit http://www.continuousphysics.com/Bullet/phpBB2/index.php
 *
 * @section install_sec Installation
 *
 * @subsection step1 Step 1: Download
 * You can download the Bullet Physics Library from our website: http://www.continuousphysics.com/Bullet/
 * @subsection step2 Step 2: Building
 * Bullet comes with autogenerated Project Files for Microsoft Visual Studio 6, 7, 7.1 and 8.
 * The main Workspace/Solution is located in Bullet/msvc/8/wksbullet.sln (replace 8 with your version).
 * 
 * Under other platforms, like Linux or Mac OS-X, Bullet can be build using either using cmake, http://www.cmake.org, or jam, http://www.perforce.com/jam/jam.html . cmake can autogenerate Xcode, KDevelop, MSVC and other build systems. just run cmake . in the root of Bullet.
 * Jam is a build system that can build the library, demos and also autogenerate the MSVC Project Files.
 * So if you are not using MSVC, you can run configure and jam .
 * If you don't have jam installed, you can make jam from the included jam-2.5 sources, or download jam from ftp://ftp.perforce.com/pub/jam/
 * 
 * @subsection step3 Step 3: Testing demos
 * Try to run and experiment with CcdPhysicsDemo executable as a starting point.
 * Bullet can be used in several ways, as Full Rigid Body simulation, as Collision Detector Library or Low Level / Snippets like the GJK Closest Point calculation.
 * The Dependencies can be seen in this documentation under Directories
 * 
 * @subsection step4 Step 4: Integrating in your application, Full Rigid Body Simulation
 * Check out CcdPhysicsDemo how to create a btDynamicsWorld, btRigidBody and btCollisionShape, Stepping the simulation and synchronizing your graphics object transform.
 * PLEASE NOTE THE CcdPhysicsEnvironment and CcdPhysicsController is obsolete and will be removed. It has been replaced by classes derived frmo btDynamicsWorld and btRididBody
 * @subsection step5 Step 5 : Integrate the Collision Detection Library (without Dynamics and other Extras)
 * Bullet Collision Detection can also be used without the Dynamics/Extras.
 * Check out btCollisionWorld and btCollisionObject, and the CollisionInterfaceDemo. Also in Extras/test_BulletOde.cpp there is a sample Collision Detection integration with Open Dynamics Engine, ODE, http://www.ode.org
 * @subsection step6 Step 6 : Use Snippets like the GJK Closest Point calculation.
 * Bullet has been designed in a modular way keeping dependencies to a minimum. The ConvexHullDistance demo demonstrates direct use of btGjkPairDetector.
 *
 * @section copyright Copyright
 * Copyright (C) 2005-2007 Erwin Coumans, some contributions Copyright Gino van den Bergen, Christer Ericson, Simon Hobbs, Ricardo Padrela, F Richter(res), Stephane Redon
 * Special thanks to all visitors of the Bullet Physics forum, and in particular above contributors, Dave Eberle, Dirk Gregorius, Erin Catto, Dave Eberle, Adam Moravanszky,
 * Pierre Terdiman, Kenny Erleben, Russell Smith, Oliver Strunk, Jan Paul van Waveren, Marten Svanfeldt.
 * 
 */
 
 

#ifndef COLLISION_WORLD_H
#define COLLISION_WORLD_H

class btStackAlloc;
class btCollisionShape;
class btConvexShape;
class btBroadphaseInterface;
#include "../../LinearMath/btVector3.h"
#include "../../LinearMath/btTransform.h"
#include "btCollisionObject.h"
#include "btCollisionDispatcher.h" //for definition of btCollisionObjectArray
#include "../BroadphaseCollision/btOverlappingPairCache.h"
#include "../../LinearMath/btAlignedObjectArray.h"

///CollisionWorld is interface and container for the collision detection
class btCollisionWorld
{

	
protected:

	btAlignedObjectArray<btCollisionObject*>	m_collisionObjects;
	
	btDispatcher*	m_dispatcher1;

	btDispatcherInfo	m_dispatchInfo;

	btStackAlloc*	m_stackAlloc;

	btOverlappingPairCache*	m_broadphasePairCache;
	
	bool	m_ownsDispatcher;
	bool	m_ownsBroadphasePairCache;

public:

	//this constructor doesn't own the dispatcher and paircache/broadphase
	btCollisionWorld(btDispatcher* dispatcher,btOverlappingPairCache* pairCache, int stackSize = 2*1024*1024);

	virtual ~btCollisionWorld();


	btBroadphaseInterface*	getBroadphase()
	{
		return m_broadphasePairCache;
	}

	btOverlappingPairCache*	getPairCache()
	{
		return m_broadphasePairCache;
	}


	btDispatcher*	getDispatcher()
	{
		return m_dispatcher1;
	}

	///LocalShapeInfo gives extra information for complex shapes
	///Currently, only btTriangleMeshShape is available, so it just contains triangleIndex and subpart
	struct	LocalShapeInfo
	{
		int	m_shapePart;
		int	m_triangleIndex;
		
		//const btCollisionShape*	m_shapeTemp;
		//const btTransform*	m_shapeLocalTransform;
	};

	struct	LocalRayResult
	{
		LocalRayResult(btCollisionObject*	collisionObject, 
			LocalShapeInfo*	localShapeInfo,
			const btVector3&		hitNormalLocal,
			btScalar hitFraction)
		:m_collisionObject(collisionObject),
		m_localShapeInfo(localShapeInfo),
		m_hitNormalLocal(hitNormalLocal),
		m_hitFraction(hitFraction)
		{
		}

		btCollisionObject*		m_collisionObject;
		LocalShapeInfo*			m_localShapeInfo;
		btVector3				m_hitNormalLocal;
		btScalar				m_hitFraction;

	};

	///RayResultCallback is used to report new raycast results
	struct	RayResultCallback
	{
		virtual ~RayResultCallback()
		{
		}
		btScalar	m_closestHitFraction;
		bool	HasHit()
		{
			return (m_closestHitFraction < btScalar(1.));
		}

		RayResultCallback()
			:m_closestHitFraction(btScalar(1.))
		{
		}
		virtual	btScalar	AddSingleResult(LocalRayResult& rayResult) = 0;
	};

	struct	ClosestRayResultCallback : public RayResultCallback
	{
		ClosestRayResultCallback(const btVector3&	rayFromWorld,const btVector3&	rayToWorld)
		:m_rayFromWorld(rayFromWorld),
		m_rayToWorld(rayToWorld),
		m_collisionObject(0)
		{
		}

		btVector3	m_rayFromWorld;//used to calculate hitPointWorld from hitFraction
		btVector3	m_rayToWorld;

		btVector3	m_hitNormalWorld;
		btVector3	m_hitPointWorld;
		btCollisionObject*	m_collisionObject;
		
		virtual	btScalar	AddSingleResult(LocalRayResult& rayResult)
		{

//caller already does the filter on the m_closestHitFraction
			assert(rayResult.m_hitFraction <= m_closestHitFraction);
			
			m_closestHitFraction = rayResult.m_hitFraction;
			m_collisionObject = rayResult.m_collisionObject;
			m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;
			m_hitPointWorld.setInterpolate3(m_rayFromWorld,m_rayToWorld,rayResult.m_hitFraction);
			return rayResult.m_hitFraction;
		}
	};


	

	int	getNumCollisionObjects() const
	{
		return int(m_collisionObjects.size());
	}

	/// rayTest performs a raycast on all objects in the btCollisionWorld, and calls the resultCallback
	/// This allows for several queries: first hit, all hits, any hit, dependent on the value returned by the callback.
	void	rayTest(const btVector3& rayFromWorld, const btVector3& rayToWorld, RayResultCallback& resultCallback, short int collisionFilterMask=-1);

	/// rayTestSingle performs a raycast call and calls the resultCallback. It is used internally by rayTest.
	/// In a future implementation, we consider moving the ray test as a virtual method in btCollisionShape.
	/// This allows more customization.
	static void	rayTestSingle(const btTransform& rayFromTrans,const btTransform& rayToTrans,
					  btCollisionObject* collisionObject,
					  const btCollisionShape* collisionShape,
					  const btTransform& colObjWorldTransform,
					  RayResultCallback& resultCallback, short int collisionFilterMask=-1);

	/// objectQuerySingle performs a collision detection query and calls the resultCallback. It is used internally by rayTest.
	static void	objectQuerySingle(const btConvexShape* castShape, const btTransform& rayFromTrans,const btTransform& rayToTrans,
					  btCollisionObject* collisionObject,
					  const btCollisionShape* collisionShape,
					  const btTransform& colObjWorldTransform,
					  RayResultCallback& resultCallback, short int collisionFilterMask=-1);

	void	addCollisionObject(btCollisionObject* collisionObject,short int collisionFilterGroup=1,short int collisionFilterMask=1);

	btCollisionObjectArray& getCollisionObjectArray()
	{
		return m_collisionObjects;
	}

	const btCollisionObjectArray& getCollisionObjectArray() const
	{
		return m_collisionObjects;
	}


	void	removeCollisionObject(btCollisionObject* collisionObject);

	virtual void	performDiscreteCollisionDetection();

	btDispatcherInfo& getDispatchInfo()
	{
		return m_dispatchInfo;
	}

};


#endif //COLLISION_WORLD_H
