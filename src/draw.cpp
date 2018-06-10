
#include <algorithm>
#include <cassert>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "draw.hpp"

static void _drawMeshPrimitive(
  ShaderProgram& shaderProgram,
  const MeshPrimitive& meshPrimitive, const Material& material,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection
) {
  assert(meshPrimitive.isLoaded());
  assert(material.isLoaded());
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

  glUniform3fv(
    shaderProgram.uniform("cc_lightPos"), 1,
    glm::value_ptr(glm::vec3(view * gc_lightPos))
  );

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, material.baseColorTexture->texId);
  glUniform1i(shaderProgram.uniform("textureId"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, material.normalMap->texId);
  glUniform1i(shaderProgram.uniform("normalMapId"), 1);

  glUniform4fv(
    shaderProgram.uniform("c_materialColor"), 1,
    glm::value_ptr(material.baseColorFactor)
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

void draw(
  ShaderProgram& shaderProgram,
  const MeshPrimitive& meshPrimitive, const Material& material,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection
) {
  shaderProgram.use();
  _drawMeshPrimitive(
    shaderProgram,
    meshPrimitive, material,
    model, view, projection
  );
  shaderProgram.disable();
}

static void _drawMesh(
  ShaderProgram& shaderProgram,
  const AssetManager& assets, size_t assetId, uint32_t meshIndex,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection
) {
  const fx::gltf::Document& document = *assets.getAsset(assetId);
  const auto& mesh = *assets.getMesh(assetId, meshIndex);
  const auto& meshObj = document.meshes[meshIndex];

  for (size_t i = 0; i < mesh.primitives.size(); ++i) {
    const MeshPrimitive& meshPrimitive = mesh.primitives[i];
    auto materialIndex = meshObj.primitives[i].material;
    auto material = materialIndex != -1
      ? *assets.getMaterial(assetId, materialIndex)
      : Material {};

    _drawMeshPrimitive(
      shaderProgram,
      meshPrimitive, material,
      model, view, projection
    );
  }
}

static glm::mat4 _getNextModel(
  const glm::mat4& model, const fx::gltf::Node& node
) {
  return (
    model *
    glm::translate(glm::mat4(1), glm::make_vec3(node.translation.data())) *
    glm::mat4_cast(glm::make_quat(node.rotation.data())) *
    glm::scale(glm::mat4(1), glm::make_vec3(node.scale.data())) *
    glm::make_mat4(node.matrix.data())
  );
}

static std::vector<fx::gltf::Node> _getAnimatedNodes(
  const AssetManager& assets, size_t assetId,
  uint32_t animIndex,
  float animTime
) {
  const fx::gltf::Document& document = *assets.getAsset(assetId);
  const fx::gltf::Animation& anim = document.animations[animIndex];

  std::vector<fx::gltf::Node> newNodes = document.nodes;

  for (const auto& channel: anim.channels) {
    fx::gltf::Node& node = newNodes[channel.target.node];
    const auto& sampler = anim.samplers[channel.sampler];

    const Accessor& times = *assets.getAccessor(assetId, sampler.input);
    const Accessor& values = *assets.getAccessor(assetId, sampler.output);

    assert(times.count == values.count);
    assert(times.type == Accessor::Type::Scalar);
    assert(times.componentType == Accessor::ComponentType::Float);

    if (times.count == 0)
      continue;

    // Keyframes
    uint32_t prevKf;
    uint32_t nextKf = 0;
    while (nextKf < times.count && times.getComponent(nextKf) < animTime) {
      nextKf++;
    }
    prevKf = std::max<uint32_t>(1, nextKf) - 1;
    nextKf = std::min<uint32_t>(nextKf, times.count - 1);

    float prevTime = times.getComponent(prevKf);
    float nextTime = times.getComponent(nextKf);
    float transition = (nextKf != prevKf) ?
      (animTime - prevTime) / (nextTime - prevTime)
      : 0;

    if (channel.target.path == "translation") {
      for (size_t i = 0; i < 3; ++i) {
        node.translation[i] = (
          values.getComponent(prevTime, i) * (1 - transition)
          + values.getComponent(nextTime, i) * transition
        );
      }
    }
    else if (channel.target.path == "rotation") {
      for (size_t i = 0; i < 4; ++i) {
        node.rotation[i] = (
          values.getComponent(prevTime, i) * (1 - transition)
          + values.getComponent(nextTime, i) * transition
        );
      }
    }
    else if (channel.target.path == "scale") {
      for (size_t i = 0; i < 3; ++i) {
        node.scale[i] = (
          values.getComponent(prevTime, i) * (1 - transition)
          + values.getComponent(nextTime, i) * transition
        );
      }
    }
  }
  return std::move(newNodes);
}

void _setSkeletonData(
  std::vector<glm::mat4>& skeletonData,
  std::vector<float>* bufferData,
  const glm::mat4& model,
  const std::vector<fx::gltf::Node>& nodes,
  const std::vector<uint32_t>& jointIndices,
  uint32_t nodeIndex
) {
  glm::mat4 nodeModel = _getNextModel(model, nodes[nodeIndex]);

  for (size_t i = 0; i < jointIndices.size(); ++i) {
    if (jointIndices[i] == nodeIndex) {
      skeletonData[i] = nodeModel;
      if (bufferData)
      {
        glm::vec4 matPos = model * glm::vec4(0, 0, 0, 1);
        (*bufferData)[i * 6 + 0] = matPos.x;
        (*bufferData)[i * 6 + 1] = matPos.y;
        (*bufferData)[i * 6 + 2] = matPos.z;

        matPos = nodeModel * glm::vec4(0, 0, 0, 1);
        (*bufferData)[i * 6 + 3] = matPos.x;
        (*bufferData)[i * 6 + 4] = matPos.y;
        (*bufferData)[i * 6 + 5] = matPos.z;
      }
      break;
    }
  }

  for (uint32_t childIndex: nodes[nodeIndex].children) {
    _setSkeletonData(
      skeletonData,
      bufferData,
      nodeModel,
      nodes,
      jointIndices,
      childIndex
    );
  }
}

std::vector<glm::mat4> _getSkeleton(
  const std::vector<fx::gltf::Node>& nodes,
  const std::vector<uint32_t>& jointIndices,
  uint32_t skeletonNodeIndex,
  std::vector<float>* bufferData
) {
  std::vector<glm::mat4> skeletonData(jointIndices.size(), glm::mat4(1));
  if (bufferData)
    *bufferData = std::vector<float>(3 * 2 * jointIndices.size(), 0);
  _setSkeletonData(
    skeletonData,
    bufferData,
    glm::mat4(1),
    nodes,
    jointIndices,
    skeletonNodeIndex
  );
  return skeletonData;
}

static void _drawNode(
  ShaderProgram& shaderProgram,
  const AssetManager& assets, size_t assetId,
  const std::vector<fx::gltf::Node>& nodes, uint32_t nodeIndex,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection
) {
  const fx::gltf::Document& document = *assets.getAsset(assetId);
  const fx::gltf::Node& node = nodes[nodeIndex];

  glm::mat4 nodeModel = _getNextModel(model, node);

  if (true && node.mesh != -1) {
    _drawMesh(
      shaderProgram,
      assets, assetId, node.mesh,
      nodeModel, view, projection
    );
  }

  if (false && node.skin != -1) {
    const fx::gltf::Skin& skin = document.skins[node.skin];

    std::vector<float> bufferData;
    _getSkeleton(nodes, skin.joints, skin.skeleton, &bufferData);
    BufferView* skeleton = createBufferView(bufferData);

    Accessor accessor = {
      skeleton,
      0,
      bufferData.size() / 3,
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
    // FIXME
    TextureData defaultColorTexture = {
      std::vector<uint8_t>(4, 255),
      1, 1,
      fx::gltf::Sampler {}
    };
    TextureData defaultNormalMap = {
      { 0, 0, 255, 255 },
      1, 1,
      fx::gltf::Sampler {}
    };
    Material mat {
      glm::vec4(1),
      &defaultColorTexture,
      &defaultNormalMap
    };
    mat.loadToGpu();

    _drawMeshPrimitive(
      shaderProgram,
      skeletonMesh, mat,
      nodeModel, view, projection
    );

    glDeleteBuffers(1, &skeleton->vboId);
  }

  for (uint32_t childIndex: nodes[nodeIndex].children) {
    _drawNode(
      shaderProgram,
      assets, assetId,
      nodes, childIndex,
      nodeModel, view, projection
    );
  }
}

void draw(
  ShaderProgram& shaderProgram,
  const AssetManager& assets, size_t assetId,
  const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
  float elapsedTime
) {
  shaderProgram.use();

  const fx::gltf::Document& document = *assets.getAsset(assetId);
  std::vector<fx::gltf::Node> nodes = document.animations.size() > 0
    ? _getAnimatedNodes(assets, assetId, 0, elapsedTime)
    : document.nodes;

  for (uint32_t rootNode: document.scenes[0].nodes) {
    _drawNode(
      shaderProgram,
      assets, assetId,
      nodes, rootNode,
      model, view, projection
    );
  }

  shaderProgram.disable();
}
