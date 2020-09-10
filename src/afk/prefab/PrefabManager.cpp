#include "afk/prefab/PrefabManager.hpp"

#include <filesystem>
#include <fstream>

#include <glm/glm.hpp>

#include "afk/debug/Assert.hpp"
#include "afk/ecs/component/Component.hpp"
#include "afk/io/Json.hpp"
#include "afk/io/JsonSerialization.hpp"
#include "afk/io/Log.hpp"
#include "afk/io/Path.hpp"
#include "afk/io/Unicode.hpp"
#include "afk/prefab/Prefab.hpp"
#include "afk/utility/Visitor.hpp"

using afk::prefab::PrefabManager;

using afk::ecs::Entity;
using afk::ecs::component::Component;
using afk::io::Json;
using afk::prefab::Prefab;
using afk::utility::Visitor;
using std::ifstream;
using std::string;
using std::filesystem::directory_iterator;
using std::filesystem::path;
using namespace afk::ecs::component;

auto PrefabManager::load_prefabs_from_dir(const path &dir_path) -> void {
  const auto prefab_dir = afk::io::get_resource_path(dir_path);
  afk_assert(std::filesystem::exists(prefab_dir),
             "Prefab directory doesn't exist");

  for (const auto &file_path : directory_iterator{prefab_dir}) {
    auto file   = ifstream{file_path};
    auto json   = Json{};
    auto prefab = Prefab{};
    file >> json;

    prefab.name     = json.at("name");
    auto components = json.at("components");

    for (const auto &[component_name, component_value] : components.items()) {
      auto component      = this->COMPONENT_MAP.at(component_name);
      auto component_json = component_value;

      auto visitor =
          Visitor{[&component_json](ModelComponent &c) { c = component_json; },
                  [&component_json](PositionComponent &c) { c = component_json; },
                  [&component_json](VelocityComponent &c) { c = component_json; },
                  [](auto) { afk_unreachable(); }};

      std::visit(visitor, component);

      afk_assert(prefab.components.find(component_name) == prefab.components.end(),
                 "Component already exists");
      this->initialize_component(components.at(component_name), component);
      prefab.components[component_name] = std::move(component);
    }

    afk_assert(this->prefab_map.find(prefab.name) == this->prefab_map.end(),
               "Prefab already exists");
    this->prefab_map[prefab.name] = std::move(prefab);
    afk::io::log << "Loaded prefab "
                 << file_path.path().lexically_relative(afk::io::get_resource_path())
                 << "\n";
  }
}

auto PrefabManager::initialize_component(const Json &json, Component &component) -> void {
  auto visitor = Visitor{[json](ModelComponent &c) {
                           auto &afk = afk::Engine::get();
                           auto path =
                               afk::io::get_resource_path(json.at("file_path"));
                           c.model_handle = afk.renderer.get_model(path);
                         },
                         [](auto) {}};

  std::visit(visitor, component);
}

auto PrefabManager::instantiate_prefab(const Prefab &prefab) const -> Entity {
  auto &afk      = afk::Engine::get();
  auto &registry = afk.ecs.registry;

  auto entity = registry.create();

  auto visitor = Visitor{[&registry, entity](ModelComponent component) {
                           registry.emplace<ModelComponent>(entity, component);
                         },
                         [&registry, entity](PositionComponent component) {
                           registry.emplace<PositionComponent>(entity, component);
                         },
                         [&registry, entity](VelocityComponent component) {
                           registry.emplace<VelocityComponent>(entity, component);
                         },
                         [](auto) { afk_unreachable(); }};

  for (const auto &[_, component] : prefab.components) {
    std::visit(visitor, component);
  }

  return entity;
}

auto PrefabManager::instantiate_prefab(const string &name) const -> Entity {
  return this->instantiate_prefab(this->prefab_map.at(name));
}

auto PrefabManager::initialize() -> void {
  afk_assert(!this->is_initialized, "Prefab manager already initialized");
  this->load_prefabs_from_dir();
  this->is_initialized = true;
}
