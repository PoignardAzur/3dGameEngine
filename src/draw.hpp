
#ifndef DRAW_H
#define DRAW_H

#include <glm/mat4x4.hpp>

#include "AssetManager.hpp"
#include "ShaderProgram.hpp"
#include "Primitives.hpp"

void draw(
  ShaderProgram& shaderProgram,
  const MeshPrimitive& meshPrimitive, const Material& material,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection
);

void draw(
  ShaderProgram& shaderProgram,
  const AssetManager& assets, size_t assetId,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
  float elapsedTime
);

#endif // !DRAW_H
