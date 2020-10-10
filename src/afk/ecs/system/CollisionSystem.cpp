#include "afk/ecs/system/CollisionSystem.hpp"

#include "afk/Engine.hpp"
#include "afk/debug/Assert.hpp"
#include "afk/io/Log.hpp"
#include "afk/utility/Visitor.hpp"

using afk::system::CollisionSystem;

CollisionSystem::CollisionSystem() {
  // Instantiate the world
  this->world = this->physics_common.createPhysicsWorld();
  // Disable gravity; ReactPhysics3D will only be used for collision detection, not applying physics
  this->world->setIsGravityEnabled(false);

  // Turn off sleeping of collisions.
  // Then bodies are colliding but aren't really changing their state for a while, they will be put to 'sleep' to stop testing collisions.
  // Putting a body to sleep will make it no longer appear to be colliding externally.
  // Temporarily, we will be disabling this optimisation for simplicity's sake, so we can process every collision
  // @todo remove the need to turn off the sleeping optimisation in ReactPhysics3D
  this->world->enableSleeping(false);

  // turn on debug renderer so ReactPhysics3D creates render data
  // @todo turn off debug ReactPhysics3D data to be generated in ReactPhysics3D when debug rendering is not being used
  this->world->setIsDebugRenderingEnabled(true);

  // Set all debug items to be displayed
  // @todo set which debug items to generate display data for in GUI
  this->world->getDebugRenderer().setIsDebugItemDisplayed(
      rp3d::DebugRenderer::DebugItem::COLLIDER_AABB, true);
  this->world->getDebugRenderer().setIsDebugItemDisplayed(
      rp3d::DebugRenderer::DebugItem::COLLIDER_BROADPHASE_AABB, true);
  this->world->getDebugRenderer().setIsDebugItemDisplayed(
      rp3d::DebugRenderer::DebugItem::COLLISION_SHAPE, true);
  this->world->getDebugRenderer().setIsDebugItemDisplayed(
      rp3d::DebugRenderer::DebugItem::CONTACT_POINT, true);
  this->world->getDebugRenderer().setIsDebugItemDisplayed(
      rp3d::DebugRenderer::DebugItem::CONTACT_NORMAL, true);

  // Set the debug logger for events in ReactPhysics3D
  this->physics_common.setLogger(&(this->logger));
}

auto CollisionSystem::Update() -> void {
  this->world->update(afk::Engine::get().get_delta_time());
}

auto CollisionSystem::LoadComponent(const afk::ecs::Entity &entity,
                                    const afk::ecs::component::ColliderComponent &collider_component)
    -> void {
  // check if entity has already had a collider component loaded
  afk_assert(
      this->ecs_entity_to_rp3d_body_map.count(entity) < 1,
      "Collider component has already being loaded for the given entity");

  const auto &t = collider_component.transform;

  // instantiate rp3d body, applying the appropriate translation and rotation
  // note that the scale is not included in rp3d transform, so collision bodies will be manually scaled later
  auto body = this->world->createCollisionBody(rp3d::Transform(
      rp3d::Vector3(t.translation.x, t.translation.y, t.translation.z),
      rp3d::Quaternion(collider_component.transform.rotation.x, t.rotation.y,
                       t.rotation.z, t.rotation.w)));

  const auto rp3d_body_id = body->getEntity().id;
  // check that the rp3d body has not already been mapped to a afk ecs entity
  afk_assert(
      this->rp3d_body_to_ecs_entity_map.count(rp3d_body_id),
      "ReactPhysics3D body has already being mapped to an AFK ECS entity");

  // add associations between rp3d body id and afk ecs entity
  this->ecs_entity_to_rp3d_body_map.insert({entity, rp3d_body_id});
  this->rp3d_body_to_ecs_entity_map.insert({rp3d_body_id, entity});

  // add all colliders to rp3d collision body
  for (const auto &collision_body : collider_component.colliders) {
    // combine collider transform scale with parent transform
    // need to apply parent scale at the shape level, as scale cannot be applied to the parent body level
    auto collision_transform = collision_body.transform;
    collision_transform.scale.x *= t.scale.x;
    collision_transform.scale.y *= t.scale.y;
    collision_transform.scale.z *= t.scale.z;

    // create transform for collider (this does NOT include scale as reactphysics does not have scale in its transform, to get around this the scale is manually added to the collision shapes)
    // todo apply parent transforms
    const auto rp3d_transform =
        rp3d::Transform(rp3d::Vector3(collision_transform.translation.x,
                                      collision_transform.translation.y,
                                      collision_transform.translation.z),
                        rp3d::Quaternion(collision_transform.rotation.x,
                                         collision_transform.rotation.y,
                                         collision_transform.rotation.z,
                                         collision_transform.rotation.w));

    // todo check transform is correct

    // add collider based on collider type
    auto visitor = afk::utility::Visitor{
        [this, &collision_transform, &rp3d_transform, &body](afk::physics::shape::Box shape) {
          // add rp3d shape and rp3d transform to collider
          // @todo create shapes when creating prefabs
          body->addCollider(this->create_shape_box(shape, collision_transform.scale),
                            rp3d_transform);
        },
        [this, &collision_transform, &rp3d_transform, &body](afk::physics::shape::Sphere shape) {
          // add rp3d shape and rp3d transform to collider
          // @todo create shapes when creating prefabs
          body->addCollider(this->create_shape_sphere(shape, collision_transform.scale),
                            rp3d_transform);
        },
        [this, &collision_transform, &rp3d_transform, &body](afk::physics::shape::Capsule shape) {
          // add rp3d shape and rp3d transform to collider
          // @todo create shapes when creating prefabs
          body->addCollider(
              this->create_shape_capsule(shape, collision_transform.scale), rp3d_transform);
        },
        [](auto) { afk_unreachable(); }};

    std::visit(visitor, collision_body.body);
  }
}

rp3d::BoxShape *CollisionSystem::create_shape_box(const afk::physics::shape::Box &box,
                                                  const glm::vec3 &scale) {
  return this->physics_common.createBoxShape(
      rp3d::Vector3(box.x * scale.x, box.y * scale.y, box.z * scale.z));
}

rp3d::SphereShape *CollisionSystem::create_shape_sphere(const afk::physics::shape::Sphere &sphere,
                                                        const glm::vec3 &scale) {
  const auto scale_factor = (scale.x + scale.y + scale.z) / 3.0f;
  return this->physics_common.createSphereShape(sphere * scale_factor);
}

rp3d::CapsuleShape *CollisionSystem::create_shape_capsule(const afk::physics::shape::Capsule &capsule,
                                                          const glm::vec3 &scale) {
  return this->physics_common.createCapsuleShape(
      capsule.radius * ((scale.x + scale.y) / 2.0f), capsule.height * scale.y);
}

void CollisionSystem::Logger::log(rp3d::Logger::Level level, const std::string &physicsWorldName,
                                  rp3d::Logger::Category category, const std::string &message,
                                  const char *filename, int lineNumber) {
  afk::io::log << "[" << getLevelName(level) << "] " << message << "\n";
}
