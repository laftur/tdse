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


bullet_components::bullet_components()
  : dispatcher(&collision_config),
  convexAlgo2d(&simplex, &pdsolver)
{
  dispatcher.registerCollisionCreateFunc(CONVEX_2D_SHAPE_PROXYTYPE,
    CONVEX_2D_SHAPE_PROXYTYPE, &convexAlgo2d);
}


bullet_world::bullet_world(bullet_components & components)
  : btDiscreteDynamicsWorld(&components.dispatcher,
    &components.overlapping_pair_cache, &components.solver,
    &components.collision_config)
{
  setGravity(btVector3(0, 0, 0));
  setInternalTickCallback(&presubstep_callback, 0, true);
}

void bullet_world::step(float_seconds time)
{
  stepSimulation(time.count(), 10);  // max 10 sub-steps
}
void bullet_world::add(body & b)
{
  b.join(*this);
}
void bullet_world::add(btTypedConstraint & c)
{
  addConstraint(&c);
}
void bullet_world::remove(body & b)
{
  b.part(*this);
}
void bullet_world::remove(btTypedConstraint & c)
{
  removeConstraint(&c);
}

void bullet_world::presubstep_callback(btDynamicsWorld * world,
  btScalar substep_time)
{
  static_cast<bullet_world &>(*world).presubstep
    ( bullet_world::float_seconds(substep_time) );
}
void bullet_world::presubstep(float_seconds substep_time)
{
  btDispatcher & dispatcher = *(getDispatcher());
  int manifolds = dispatcher.getNumManifolds();
  for(int i = 0; i != manifolds; ++i)
  {
    btPersistentManifold & manifold
      = *(dispatcher.getManifoldByIndexInternal(i));

    int contacts = manifold.getNumContacts();
    for(int contact = 0; contact != contacts; ++contact)
    {
      if(manifold.getContactPoint(contact).getDistance() <= 0.0)
      {
        // It's safe to modify bodies in the substep callback.
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

  btCollisionObjectArray & array = getCollisionObjectArray();
  for(int i = 0; i < array.size(); ++i)
  {
    if(needs_presubstep * ptr =
      dynamic_cast<needs_presubstep *>( static_cast<body *>(array[i]) )
    ) ptr->presubstep(substep_time);
  }
}


motion_state::motion_state( const glm::vec2 & position,
  const glm::mat2 & orientation = glm::mat2(1.0) )
  : transform(
      btMatrix3x3(
        orientation[0][0], orientation[1][0], 0.0,
        orientation[0][1], orientation[1][1], 0.0,
        0.0, 0.0, 1.0
      ),
      btVector3(position.x, position.y, 0.0)
    )
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
  ( btScalar mass, btMotionState & state, const btCollisionShape & shape,
  const btVector3 & inertia = btVector3(0, 0, 0) )
{
  btRigidBody::btRigidBodyConstructionInfo _info(
    mass, &state,
    // Shapes can be shared between btRigidBody objects.
    const_cast<btCollisionShape *>(&shape),
    inertia
  );
  _info.m_rollingFriction = 0.2;

  return _info;
}
btVector3 body::calc_local_inertia(const btCollisionShape & shape, float mass)
{
  btVector3 inertia;
  shape.calculateLocalInertia(mass, inertia);
  return inertia;
}


#include <glm/gtc/matrix_transform.hpp>
body::body(float mass, const btCollisionShape & shape,
  const glm::vec2 & _position)
  : motion_state(_position, glm::mat2
     ( glm::rotate(glm::mat4(1.0), 0.0f, glm::vec3(0.0, 0.0, 1.0)) )
  ),
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

void body::join(btDiscreteDynamicsWorld & world)
{
  world.addRigidBody(this);
}
void body::part(btDiscreteDynamicsWorld & world)
{
  world.removeRigidBody(this);
}
