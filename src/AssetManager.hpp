
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
  void gpuLoadAll(const MeshPrimitive::AttributeMap& attributeMap);

  const Mesh* getMesh(const std::string& assetPath, size_t meshIndex) const;
  const Mesh* getMesh(size_t assetId, size_t meshIndex) const;

  const fx::gltf::Document* getAsset(size_t assetId) const;

private:
  void loadMesh(size_t assetId, size_t meshIndex);

  std::unordered_map<size_t, fx::gltf::Document> m_assets;
  std::unordered_map<std::string, size_t> m_assetPaths;
  size_t m_nextAssetId = 0;

  std::unordered_map<size_t, std::vector<std::optional<Mesh>>> m_meshes;
  std::unordered_map<size_t, std::vector<std::optional<Accessor>>> m_accessors;
  std::unordered_map<size_t, std::vector<std::optional<BufferView>>> m_bufferViews;
  std::unordered_map<size_t, std::vector<std::optional<BufferData>>> m_buffers;
};

#endif // !ASSET_MANAGER_H

struct Mesh;
struct Accessor;
struct BufferView;
struct BufferData;
