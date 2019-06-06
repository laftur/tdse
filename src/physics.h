/*
Copyright (C) 2016 Jeremy Starnes

This file is part of TDSE.

TDSE is free software: you can redistribute it and/or modify it under the terms
of the GNU Affero General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

TDSE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along
with TDSE; see the file COPYING. If not, see <http://www.gnu.org/licenses/agpl>
*/
#ifndef PHYSICS_H_INCLUDED
#define PHYSICS_H_INCLUDED


#include "glm.h"
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.h>
#include <BulletCollision/CollisionShapes/btConvex2dShape.h>
#include <BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.h>
#include <LinearMath/btGeometryUtil.h>


template<class T> btConvexHullShape make_convex_hull(const T & vertices)
{
  btAlignedObjectArray<btVector3> _vertices;
  _vertices.reserve(vertices.size()*2);
  for(auto i = vertices.begin(); i != vertices.end(); ++i)
    _vertices.push_back( btVector3(i->x, i->y, 1.0f) );
  for(auto i = vertices.begin(); i != vertices.end(); ++i)
    _vertices.push_back( btVector3(i->x, i->y, -1.0f) );

  btAlignedObjectArray<btVector3> planeEquations;
  btGeometryUtil::getPlaneEquationsFromVertices(_vertices, planeEquations);

  btAlignedObjectArray<btVector3> shiftedPlaneEquations;
  for (int p=0;p<planeEquations.size();p++)
  {
    btVector3 plane = planeEquations[p];
    plane[3] += CONVEX_DISTANCE_MARGIN;
    shiftedPlaneEquations.push_back(plane);
  }
  btAlignedObjectArray<btVector3> shiftedVertices;
  btGeometryUtil::getVerticesFromPlaneEquations(shiftedPlaneEquations,
    shiftedVertices);

  return btConvexHullShape(&shiftedVertices[0].getX(),
    shiftedVertices.size());
}

glm::mat2 bt_to_glm2d(const btMatrix3x3 & btmat);
glm::mat3 bt_to_glm2d(const btTransform & bttrans);
btTransform glm2d_to_bt(const glm::mat3 & glmtrans);


class bullet_world;
class bullet_components
{
public:
  bullet_components();
private:
  friend class bullet_world;
  // collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
  btDefaultCollisionConfiguration collision_config;
  btCollisionDispatcher dispatcher;
  // btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
  btDbvtBroadphase overlapping_pair_cache;
  // the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
  btSequentialImpulseConstraintSolver solver;

  // vaguely guessing from the Box2dDemo that this stuff is needed for collision between 2d shapes to work
  btVoronoiSimplexSolver simplex;
  btMinkowskiPenetrationDepthSolver pdsolver;
  btConvex2dConvex2dAlgorithm::CreateFunc convexAlgo2d;
};


#include <chrono>
#include <set>
typedef std::chrono::duration< float, std::ratio<1> > float_seconds;
class needs_presubstep;
class body;
class bullet_world : public bullet_components, public btDiscreteDynamicsWorld
{
public:
  bullet_world();
  bullet_world(const bullet_world &) = delete;
  void operator = (const bullet_world &) = delete;

  static const float_seconds fixed_substep;
  virtual void step(float_seconds step_time);
  virtual void presubstep(float_seconds substep_time);

  void add_callback(needs_presubstep & callback);
  void remove_callback(needs_presubstep & callback);
  void add_body(body & b);
  void remove_body(body & b);

private:
  std::set<needs_presubstep *> presubsteps;
  void internalSingleStepSimulation(btScalar timeStep) override;
};


class motion_state : public btMotionState
{
public:
  motion_state(const glm::mat3 & transform_);

  glm::mat3 model() const;
  glm::mat2 orientation() const;
  glm::vec2 position() const;

private:
  btTransform transform;

  void getWorldTransform(btTransform & worldTrans) const override;
  void setWorldTransform(const btTransform & worldTrans) override;
};


class body : public motion_state, public btRigidBody
{
private:
  static btRigidBody::btRigidBodyConstructionInfo info
  (btScalar mass,
   btMotionState & state,
   const btCollisionShape & shape,
   const btVector3 & inertia);
  static btVector3 calc_local_inertia(const btCollisionShape & shape,
                                      float mass);

public:
  body(float mass,
       const btCollisionShape & cs,
       const glm::mat3 & transform);

  glm::mat3 real_transform() const;
  glm::mat2 real_orientation() const;
  glm::vec2 real_position() const;

  void warp(const glm::mat3 & new_trans);
};


class needs_collision
{
public:
  virtual void collision(body & b) = 0;
};
class needs_presubstep
{
public:
  virtual void presubstep(bullet_world & world, float_seconds substep_time) = 0;
};


#endif  // PHYSICS_H_INCLUDED
