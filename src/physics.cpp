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
#include "physics.h"


glm::mat2 bt_to_glm2d(const btMatrix3x3 & btmat)
{
  btVector3 cols[2] = {btmat.getColumn(0), btmat.getColumn(1)};
  return glm::mat2(
    cols[0].getX(), cols[0].getY(),
    cols[1].getX(), cols[1].getY()
  );
}
glm::mat3 bt_to_glm2d(const btTransform & bttrans)
{
  return compose_transform(
    glm::vec2( bttrans.getOrigin().getX(), bttrans.getOrigin().getY() ),
    bt_to_glm2d( bttrans.getBasis() )
  );
}
btTransform glm2d_to_bt(const glm::mat3 & glmtrans)
{
  return btTransform(
    btMatrix3x3(
      glmtrans[0][0], glmtrans[1][0], 0.0,
      glmtrans[0][1], glmtrans[1][1], 0.0,
      0.0,            0.0,            1.0
    ),
    btVector3(glmtrans[2].x, glmtrans[2].y, 0.0)
  );
}


bullet_components::bullet_components()
  : dispatcher(&collision_config),
  convexAlgo2d(&simplex, &pdsolver)
{
  dispatcher.registerCollisionCreateFunc(CONVEX_2D_SHAPE_PROXYTYPE,
    CONVEX_2D_SHAPE_PROXYTYPE, &convexAlgo2d);
}


bullet_world::bullet_world()
  : btDiscreteDynamicsWorld(&dispatcher, &overlapping_pair_cache, &solver,
                            &collision_config)
{
  setGravity(btVector3(0, 0, 0));
}

const float_seconds bullet_world::fixed_substep(1.0f/60.0f);
void bullet_world::step(float_seconds step_time)
{
  // Limit to 10 substeps
  stepSimulation( step_time.count(), 10, fixed_substep.count() );
}
void bullet_world::presubstep(float_seconds substep_time)
{
  // Trigger all collision callbacks
  btDispatcher & dispatcher = *( getDispatcher() );
  int manifolds = dispatcher.getNumManifolds();
  for(int i = 0; i != manifolds; ++i)
  {
    btPersistentManifold & manifold =
      *( dispatcher.getManifoldByIndexInternal(i) );

    int contacts = manifold.getNumContacts();
    for(int contact = 0; contact != contacts; ++contact)
    {
      if(manifold.getContactPoint(contact).getDistance() <= 0.0)
      {
        // All btCollisionObect instances are assumed to be body instances
        // It's safe to modify bodies between substeps
        body * body0 = static_cast<body *>
          ( const_cast<btCollisionObject *>(manifold.getBody0()) );
        body * body1 = static_cast<body *>
          ( const_cast<btCollisionObject *>(manifold.getBody1()) );

        needs_collision * ptr;
        if( (ptr = dynamic_cast<needs_collision *>(body0)) )
          ptr->collision(*body1);
        if( (ptr = dynamic_cast<needs_collision *>(body1)) )
          ptr->collision(*body0);

        break;
      }
    }
  }

  // Trigger all presubstep callbacks
  for(auto i = presubsteps.begin(); i != presubsteps.end(); ++i)
    (*i)->presubstep( *this, substep_time );

  // Step physics world
  btDiscreteDynamicsWorld::internalSingleStepSimulation( substep_time.count() );
}

void bullet_world::add_callback(needs_presubstep & callback)
{
  presubsteps.insert(&callback);
}
void bullet_world::remove_callback(needs_presubstep & callback)
{
  presubsteps.erase(&callback);
}
void bullet_world::add_body(body & b)
{
  addRigidBody(&b);
}
void bullet_world::remove_body(body & b)
{
  removeRigidBody(&b);
}

void bullet_world::internalSingleStepSimulation(btScalar timeStep)
{
  presubstep( float_seconds(timeStep) );
}


motion_state::motion_state(const glm::mat3 & transform_)
: transform( glm2d_to_bt(transform_) )
{}

glm::mat3 motion_state::model() const
{
  const btMatrix3x3 & ori = transform.getBasis();
  const btVector3 & pos = transform.getOrigin();
  btVector3 cols[3] = {ori.getColumn(0), ori.getColumn(1), ori.getColumn(2)};
  return glm::mat3(
    cols[0].getX(), cols[0].getY(), 0.0f,
    cols[1].getX(), cols[1].getY(), 0.0f,
    pos.getX(), pos.getY(), 1.0f
  );
}
glm::mat2 motion_state::orientation() const
{
  return bt_to_glm2d( transform.getBasis() );
}
glm::vec2 motion_state::position() const
{
  const btVector3 & pos = transform.getOrigin();
  return glm::vec2(pos.getX(), pos.getY());
}

void motion_state::getWorldTransform(btTransform & world_trans) const
{
  world_trans = transform;
}
void motion_state::setWorldTransform(const btTransform & world_trans)
{
  transform = world_trans;
}


btRigidBody::btRigidBodyConstructionInfo body::info
(btScalar mass,
 btMotionState & state,
 const btCollisionShape & shape,
 const btVector3 & inertia)
{
  btRigidBody::btRigidBodyConstructionInfo _info(
    mass, &state,
    // Bodies never modify collision shapes, so I dunno why this isn't const
    const_cast<btCollisionShape *>(&shape),
    inertia
  );
  _info.m_rollingFriction = 0.2;

  return _info;
}
btVector3 body::calc_local_inertia
(
  const btCollisionShape & shape,
  float mass
)
{
  btVector3 inertia;
  shape.calculateLocalInertia(mass, inertia);
  return inertia;
}


#include <glm/gtc/matrix_transform.hpp>
body::body(float mass,
           const btCollisionShape & shape,
           const glm::mat3 & transform)
: motion_state(transform),
  btRigidBody( info(
    mass, *this, shape,
    calc_local_inertia(shape, mass)
  ) )
{
  // Restrict linear movement to the XY plane
  setLinearFactor(btVector3(1, 1, 0));
  // Restrict angular movement to the z axis
  setAngularFactor(btVector3(0, 0, 1));
}

glm::mat3 body::real_transform() const
{
  return bt_to_glm2d( btRigidBody::getWorldTransform() );
}
glm::mat2 body::real_orientation() const
{
  return bt_to_glm2d( btRigidBody::getWorldTransform().getBasis() );
}
glm::vec2 body::real_position() const
{
  const btVector3 & origin = btRigidBody::getWorldTransform().getOrigin();
  return glm::vec2( origin.getX(), origin.getY() );
}

void body::warp(const glm::mat3 & new_trans)
{
  btRigidBody::setWorldTransform( glm2d_to_bt(new_trans) );
}
