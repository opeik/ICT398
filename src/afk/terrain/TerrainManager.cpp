#include "afk/terrain/TerrainManager.hpp"

#include <array>
#include <cmath>
#include <utility>
#include <vector>

#include <FastNoiseSIMD/FastNoiseSIMD.h>
#include <glm/glm.hpp>

#include "afk/debug/Assert.hpp"
#include "afk/renderer/Mesh.hpp"

using std::vector;

using glm::vec2;
using glm::vec3;

using Afk::Mesh;
using Afk::Model;
using Afk::TerrainManager;
using Index = Mesh::Index;

auto TerrainManager::generate_height_map(int width, int length, float roughness,
                                         float scaling) -> void {
  afk_assert(width >= 1, "Invalid width");
  afk_assert(length >= 1, "Invalid length");

  const auto w = width;
  const auto l = length;

  const auto num_vertices = static_cast<size_t>(w * l);

  auto *noise = FastNoiseSIMD::NewFastNoiseSIMD();
  noise->SetFrequency(roughness);
  auto *noise_set = noise->GetSimplexFractalSet(0, 0, 0, w, 1, l);
  auto index      = size_t{0};

  this->height.vertices.resize(num_vertices);

  auto vertexIndex = size_t{0};
  for (auto y = 0; y < l; ++y) {
    for (auto x = 0; x < w; ++x) {
      this->mesh.vertices[vertexIndex++].position =
          vec3{static_cast<float>(x), noise_set[index] * scaling, static_cast<float>(y)};
      ++index;
    }
  }

  FastNoiseSIMD::FreeNoiseSet(noise_set);
}

auto TerrainManager::generate_flat_plane(int width, int length) -> void {
  afk_assert(width >= 1, "Invalid width");
  afk_assert(length >= 1, "Invalid length");

  const auto w = width;
  const auto l = length;

  const auto num_vertices = static_cast<size_t>(w * l);
  const auto num_indices  = static_cast<size_t>((w - 1) * (l - 1) * 6);

  this->mesh.vertices.resize(num_vertices);
  this->mesh.indices.resize(num_indices);

  auto vertexIndex = size_t{0};
  for (auto y = 0; y < l; ++y) {
    for (auto x = 0; x < w; ++x) {
      this->mesh.vertices[vertexIndex++].position =
          vec3{static_cast<float>(x), 0.0f, static_cast<float>(y)};
    }
  }

  auto indicesIndex = size_t{0};
  for (auto y = 0; y < (l - 1); ++y) {
    for (auto x = 0; x < (w - 1); ++x) {
      auto start = y * w + x;

      this->mesh.indices[indicesIndex++] = static_cast<Mesh::Index>(start);
      this->mesh.indices[indicesIndex++] = static_cast<Mesh::Index>(start + 1);
      this->mesh.indices[indicesIndex++] = static_cast<Mesh::Index>(start + w);

      this->mesh.indices[indicesIndex++] = static_cast<Mesh::Index>(start + 1);
      this->mesh.indices[indicesIndex++] = static_cast<Mesh::Index>(start + 1 + w);
      this->mesh.indices[indicesIndex++] = static_cast<Mesh::Index>(start + w);
    }
  }
}

auto TerrainManager::initialize() -> void {
  afk_assert(!this->is_initialized, "Terrain manager already initialized");

  // afk_assert(width >= 1, "Invalid width");
  // afk_assert(length >= 1, "Invalid length");

  // this->generate_flat_plane(width, length);
  // this->generate_height_map(width, length, roughness, scaling);

  // for (auto i = std::size_t{0}; i < this->height.vertices.size(); ++i) {
  //   this->mesh.vertices[i].position.y += this->height.vertices[i].position.y;
  // }

  this->is_initialized = true;
}