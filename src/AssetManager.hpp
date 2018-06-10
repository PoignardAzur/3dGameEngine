
#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <fx/gltf.h>

#include "Primitives.hpp"

class AssetManager {
public:
  AssetManager();
  AssetManager(const AssetManager&) = delete;
  AssetManager(AssetManager&&) = delete;
  ~AssetManager();

  size_t loadAsset(const std::string& path, bool loadAll = true, bool reload = false);
  // size_t loadRawData(const std::string& path, size_t byteLength = -1, bool reload = false);
  //void loadImageData(std::string_view path);

  void gpuLoadAll(const MeshPrimitive::AttributeMap& attributeMap);

  const Mesh* getMesh(const std::string& assetPath, size_t meshIndex) const;
  const Mesh* getMesh(size_t assetId, size_t meshIndex) const;

  const Material* getMaterial(const std::string& assetPath, size_t materialIndex) const;
  const Material* getMaterial(size_t assetId, size_t materialIndex) const;

  const Accessor* getAccessor(const std::string& assetPath, size_t accessorIndex) const;
  const Accessor* getAccessor(size_t assetId, size_t accessorIndex) const;

  const fx::gltf::Document* getAsset(size_t assetId) const;

private:
  void loadMesh(size_t assetId, size_t meshIndex);
  void loadMaterial(size_t assetId, size_t materialIndex);

  std::unordered_map<size_t, fx::gltf::Document> m_assets;
  std::unordered_map<std::string, size_t> m_assetPaths;
  size_t m_nextAssetId = 0;

  std::unordered_map<size_t, std::vector<std::optional<Mesh>>> m_meshes;
  std::unordered_map<size_t, std::vector<std::optional<Material>>> m_materials;
  std::unordered_map<size_t, std::vector<std::optional<TextureData>>> m_textures;
  std::unordered_map<size_t, std::vector<std::optional<Accessor>>> m_accessors;
  std::unordered_map<size_t, std::vector<std::optional<BufferView>>> m_bufferViews;
  std::unordered_map<size_t, std::vector<std::optional<BufferData>>> m_buffers;

  TextureData m_defaultColorTexture;
  TextureData m_defaultNormalMap;
};

#endif // !ASSET_MANAGER_H
