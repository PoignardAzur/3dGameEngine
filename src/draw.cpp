
#include <algorithm>
#include <cassert>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "draw.hpp"

static void _drawMesh(
  ShaderProgram& shaderProgram,
  const MeshPrimitive& meshPrimitive,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection
) {
  assert(meshPrimitive.isLoaded());
  assert(meshPrimitive.attributes.count("POSITION") > 0);

  glBindVertexArray(meshPrimitive.vaoId);

  glUniformMatrix4fv(
    shaderProgram.uniform("mvp"),
    1, GL_FALSE,
    glm::value_ptr(projection * view * model)
  );
  glUniformMatrix4fv(
    shaderProgram.uniform("modelView"),
    1, GL_FALSE,
    glm::value_ptr(view * model)
  );
  glUniformMatrix3fv(
    shaderProgram.uniform("normalMatrix"),
    1, GL_FALSE,
    glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(view * model))))
  );

  glm::vec4 gc_lightPos(1, 2, 3, 1);

  glUniform4fv(
    shaderProgram.uniform("cc_lightPos"), 1,
    glm::value_ptr(view * gc_lightPos)
  );

  if (meshPrimitive.indices) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshPrimitive.indices->bufferView->vboId);
    glDrawElements(
      (GLenum)meshPrimitive.mode,
      meshPrimitive.indices->count,
      (GLenum)meshPrimitive.indices->componentType,
      reinterpret_cast<GLvoid*>(meshPrimitive.indices->byteOffset)
    );
  }
  else {
    glDrawArrays(
      (GLenum)meshPrimitive.mode,
      0,
      meshPrimitive.attributes.at("POSITION")->count
    );
  }

  glBindVertexArray(0);
}

static void _drawMesh(
  ShaderProgram& shaderProgram,
  const Mesh& mesh,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection
) {
  for (const MeshPrimitive& meshPrimitive: mesh.primitives) {
    _drawMesh(shaderProgram, meshPrimitive, model, view, projection);
  }
}

void draw(
  ShaderProgram& shaderProgram,
  const MeshPrimitive& meshPrimitive,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection
) {
  shaderProgram.use();
  _drawMesh(shaderProgram, meshPrimitive, model, view, projection);
  shaderProgram.disable();
}

void _setSkeletonData(
  std::vector<float>& bufferData, const glm::mat4& model,
  const AssetManager& assets, size_t assetId,
  uint32_t nodeIndex, uint32_t skinIndex
) {
  const fx::gltf::Document& document = *assets.getAsset(assetId);
  const fx::gltf::Node& node = document.nodes[nodeIndex];
  const fx::gltf::Skin& skin = document.skins[skinIndex];

  glm::mat4 nodeModel = (
    model *
    glm::translate(glm::mat4(1), glm::make_vec3(node.translation.data())) *
    glm::mat4_cast(glm::make_quat(node.rotation.data())) *
    glm::scale(glm::mat4(1), glm::make_vec3(node.scale.data())) *
    glm::make_mat4(node.matrix.data())
  );

  for (size_t i = 0; i < skin.joints.size(); ++i) {
    if (skin.joints[i] == nodeIndex) {
      glm::vec4 matPos = model * glm::vec4(0, 0, 0, 1);
      bufferData[i * 6 + 0] = matPos.x;
      bufferData[i * 6 + 1] = matPos.y;
      bufferData[i * 6 + 2] = matPos.z;

      matPos = nodeModel * glm::vec4(0, 0, 0, 1);
      bufferData[i * 6 + 3] = matPos.x;
      bufferData[i * 6 + 4] = matPos.y;
      bufferData[i * 6 + 5] = matPos.z;

      break;
    }
  }

  for (uint32_t childIndex: document.nodes[nodeIndex].children) {
    _setSkeletonData(
      bufferData, nodeModel,
      assets, assetId, childIndex, skinIndex
    );
  }
}

std::vector<float> _getSkeleton(
  const AssetManager& assets, size_t assetId,
  uint32_t skinIndex
) {
  const fx::gltf::Document& document = *assets.getAsset(assetId);
  const fx::gltf::Skin& skin = document.skins[skinIndex];

  std::vector<float> bufferData(3 * 2 * skin.joints.size(), 0);
  _setSkeletonData(bufferData, glm::mat4(1), assets, assetId, skin.skeleton, skinIndex);

  return bufferData;
}

static void _drawNode(
  ShaderProgram& shaderProgram,
  const AssetManager& assets, size_t assetId, uint32_t nodeIndex,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection
) {
  const fx::gltf::Document& document = *assets.getAsset(assetId);
  const fx::gltf::Node& node = document.nodes[nodeIndex];

  glm::mat4 nodeModel = (
    model *
    glm::translate(glm::mat4(1), glm::make_vec3(node.translation.data())) *
    glm::mat4_cast(glm::make_quat(node.rotation.data())) *
    glm::scale(glm::mat4(1), glm::make_vec3(node.scale.data())) *
    glm::make_mat4(node.matrix.data())
  );

  if (node.mesh != -1) {
    _drawMesh(
      shaderProgram, *assets.getMesh(assetId, node.mesh),
      nodeModel, view, projection
    );
  }

  if (node.skin != -1) {
    std::vector<float> skeletonData = _getSkeleton(assets, assetId, node.skin);
    BufferView* skeleton = createBufferView(skeletonData);
    Accessor accessor = {
      skeleton,
      0,
      skeletonData.size() / 3,
      Accessor::Type::Vec3,
      Accessor::ComponentType::Float,
    };
    MeshPrimitive skeletonMesh = {
      MeshPrimitive::Mode::Lines,
      { { "POSITION", &accessor } }
    };
    skeletonMesh.loadToGpu(
      { { "POSITION", 0 } }
    );

    _drawMesh(
      shaderProgram, skeletonMesh,
      nodeModel, view, projection
    );

    glDeleteBuffers(1, &skeleton->vboId);
  }

  for (uint32_t childIndex: document.nodes[nodeIndex].children) {
    _drawNode(
      shaderProgram, assets, assetId, childIndex,
      nodeModel, view, projection
    );
  }
}

void draw(
  ShaderProgram& shaderProgram,
  const AssetManager& assets, size_t assetId,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection
) {
  shaderProgram.use();

  const fx::gltf::Document& document = *assets.getAsset(assetId);

  for (uint32_t rootNode: document.scenes[0].nodes) {
    _drawNode(shaderProgram, assets, assetId, rootNode, model, view, projection);
  }

  shaderProgram.disable();
}
