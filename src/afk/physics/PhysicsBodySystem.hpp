#pragma once

#include <reactphysics3d.h>

#include <entt/entt.hpp>

#include "afk/physics/PhysicsBody.hpp"
#include "glm/vec3.hpp"

namespace Afk {
  class PhysicsBody;

  using World = rp3d::DynamicsWorld;

  class PhysicsBodySystem {
    public:
      PhysicsBodySystem();

      explicit PhysicsBodySystem(glm::vec3 gravity);

      auto get_gravity();

      auto set_gravity(glm::vec3 gravity);

      auto update(entt::registry* registry, float dt) -> void;

    private:
      World* world;

    friend class PhysicsBody;
  };
};