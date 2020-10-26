#include "afk/ecs/system/AffordanceSystem.hpp"

#include "afk/Engine.hpp"
#include "afk/ecs/component/AffordanceComponent.hpp"
#include "afk/ecs/component/NeedsComponent.hpp"
#include "afk/io/Log.hpp"
#include "afk/io/Path.hpp"
using afk::ecs::component::AffordanceComponent;
using afk::ecs::component::NeedsComponent;
using afk::ecs::system::AffordanceSystem;
auto AffordanceSystem::update() -> void {
  auto &afk       = afk::Engine::get();
  auto &registry  = afk.ecs.registry;
  const auto view = registry.view<NeedsComponent>();
  for (const auto entity : view) {
    auto &affordances = registry.get<NeedsComponent>(entity);
  }
}
