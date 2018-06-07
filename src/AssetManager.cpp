#include "AssetManager.hpp"

AssetManager::AssetManager() {}

AssetManager::~AssetManager() {
  // TODO - unload
}

size_t AssetManager::loadAsset(const std::string& path, bool loadAll, bool reload) {
  m_assetPaths[path] = m_nextAssetId;
  m_assets[m_nextAssetId] = fx::gltf::LoadFromText(path);

  const fx::gltf::Document& document = m_assets[m_nextAssetId];

  m_meshes[m_nextAssetId].resize(document.meshes.size());
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

  return m_nextAssetId++;
}

void AssetManager::gpuLoadAll(const MeshPrimitive::AttributeMap& attributeMap) {
  for (auto& pair: m_meshes) {
    auto& meshes = pair.second;
    for (auto& optMesh: meshes) {
      if (optMesh) {
        for (MeshPrimitive& primitive: optMesh->primitives) {
          primitive.loadToGpu(attributeMap);
        }
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

const fx::gltf::Document* AssetManager::getAsset(size_t assetId) const {
  auto it = m_assets.find(assetId);
  return (it != m_assets.end()) ? &it->second : nullptr;
}
