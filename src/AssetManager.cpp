#include <cassert>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

#include "AssetManager.hpp"

AssetManager::AssetManager() {
  m_defaultColorTexture = {
    std::vector<uint8_t>(4, 255),
    1, 1,
    fx::gltf::Sampler {}
  };
}

AssetManager::~AssetManager() {
  // TODO - unload
}

size_t AssetManager::loadAsset(const std::string& path, bool loadAll, bool reload) {
  m_assetPaths[path] = m_nextAssetId;
  m_assets[m_nextAssetId] = fx::gltf::LoadFromBinary(path);

  const fx::gltf::Document& document = m_assets[m_nextAssetId];

  m_meshes[m_nextAssetId].resize(document.meshes.size());
  m_materials[m_nextAssetId].resize(document.materials.size());
  m_textures[m_nextAssetId].resize(document.textures.size());
  m_accessors[m_nextAssetId].resize(document.accessors.size());
  m_bufferViews[m_nextAssetId].resize(document.bufferViews.size());
  m_buffers[m_nextAssetId].resize(document.buffers.size());

  auto& buffers = m_buffers[m_nextAssetId];
  for (size_t i = 0; i < document.buffers.size(); ++i) {
    buffers[i] = BufferData {
      document.buffers[i].data
    };
  }

  auto& bufferViews = m_bufferViews[m_nextAssetId];
  for (size_t i = 0; i < document.bufferViews.size(); ++i) {
    const fx::gltf::BufferView& bufferViewData = document.bufferViews[i];
    bufferViews[i] = BufferView {
      &*buffers[bufferViewData.buffer],
      bufferViewData.byteOffset,
      bufferViewData.byteLength,
      bufferViewData.byteStride
    };
  }

  auto& accessors = m_accessors[m_nextAssetId];
  for (size_t i = 0; i < document.accessors.size(); ++i) {
    const fx::gltf::Accessor& accessorData = document.accessors[i];
    accessors[i] = Accessor {
      &*bufferViews[accessorData.bufferView],
      accessorData.byteOffset,
      accessorData.count,
      accessorData.type,
      accessorData.componentType,
      accessorData.normalized
    };
  }

  auto& meshes = m_meshes[m_nextAssetId];
  for (size_t i = 0; i < document.meshes.size(); ++i) {
    const fx::gltf::Mesh& meshData = document.meshes[i];
    meshes[i] = Mesh {};
    meshes[i]->primitives.resize(meshData.primitives.size());

    for (size_t j = 0; j < meshData.primitives.size(); ++j) {
      const fx::gltf::Primitive& meshPrimitiveData = meshData.primitives[j];
      MeshPrimitive& meshPrimitive = meshes[i]->primitives[j];

      meshPrimitive.mode = meshPrimitiveData.mode;

      for (const auto& pair: meshPrimitiveData.attributes) {
        meshPrimitive.attributes[pair.first] = &*accessors[pair.second];
      }

      if (meshPrimitiveData.indices != -1) {
        meshPrimitive.indices = &*accessors[meshPrimitiveData.indices];
      }
    }
  }

  auto& textures = m_textures[m_nextAssetId];
  for (size_t i = 0; i < document.textures.size(); ++i) {
    const fx::gltf::Texture& textureObj = document.textures[i];
    const fx::gltf::Image& imageData = document.images[textureObj.source];

    std::vector<uint8_t> textureData;
    int x, y;
    if (imageData.uri != "") {
      uint8_t* rawData = stbi_load(imageData.uri.c_str(), &x, &y, nullptr, 4);
      assert(rawData);
      // TODO - if (!rawData)
      textureData = std::vector<uint8_t>(rawData, rawData + x * y * 4);
      stbi_image_free(rawData);
    }
    else {
      const BufferView& bufferedImage = *bufferViews[imageData.bufferView];

      assert(bufferedImage.byteStride == 0);
      uint8_t* rawData = stbi_load_from_memory(
        bufferedImage.buffer->data.data() + bufferedImage.byteOffset,
        bufferedImage.byteLength,
        &x, &y, nullptr, 4
      );
      assert(rawData);
      // TODO - if (!rawData)
      textureData = std::vector<uint8_t>(rawData, rawData + x * y * 4);
      stbi_image_free(rawData);
    }

    int32_t samplerId = textureObj.sampler;

    textures[i] = TextureData {
      std::move(textureData), x, y,
      samplerId != (int32_t)(-1)
        ? document.samplers[samplerId]
        : fx::gltf::Sampler {}
    };
  }

  auto& materials = m_materials[m_nextAssetId];
  for (size_t i = 0; i < document.materials.size(); ++i) {
    const fx::gltf::Material& materialData = document.materials[i];

    int32_t textureId = materialData.pbrMetallicRoughness.baseColorTexture.index;

    materials[i] = Material {
      glm::make_vec4(materialData.pbrMetallicRoughness.baseColorFactor.data()),
      textureId != -1 ? &*textures[textureId] : &m_defaultColorTexture
    };
  }

  return m_nextAssetId++;
}

void AssetManager::gpuLoadAll(const MeshPrimitive::AttributeMap& attributeMap) {
  for (auto& meshesIt: m_meshes) {
    for (auto& optMesh: meshesIt.second) {
      if (optMesh) {
        for (MeshPrimitive& primitive: optMesh->primitives) {
          primitive.loadToGpu(attributeMap);
        }
      }
    }
  }
  for (auto& materialsIt: m_materials) {
    for (auto& optMaterial: materialsIt.second) {
      if (optMaterial) {
        optMaterial->loadToGpu();
      }
    }
  }
}

const Mesh* AssetManager::getMesh(const std::string& assetPath, size_t meshIndex) const {
  auto it = m_assetPaths.find(assetPath);
  return (it != m_assetPaths.end()) ? getMesh(it->second, meshIndex) : nullptr;
}

const Mesh* AssetManager::getMesh(size_t assetId, size_t meshIndex) const {
  auto it = m_meshes.find(assetId);
  if (it == m_meshes.end()) {
    return nullptr;
  }
  if (!it->second[meshIndex]) {
    return nullptr;
  }
  else {
    return &*it->second[meshIndex];
  }
}

const Material* AssetManager::getMaterial(const std::string& assetPath, size_t materialIndex) const {
  auto it = m_assetPaths.find(assetPath);
  return (it != m_assetPaths.end())
    ? getMaterial(it->second, materialIndex)
    : nullptr;
}

const Material* AssetManager::getMaterial(size_t assetId, size_t materialIndex) const {
  auto it = m_materials.find(assetId);
  if (it == m_materials.end()) {
    return nullptr;
  }
  if (!it->second[materialIndex]) {
    return nullptr;
  }
  else {
    return &*it->second[materialIndex];
  }
}

const fx::gltf::Document* AssetManager::getAsset(size_t assetId) const {
  auto it = m_assets.find(assetId);
  return (it != m_assets.end()) ? &it->second : nullptr;
}
