
#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <GL/glew.h>
#include <fx/gltf.h>
#include <glm/vec4.hpp>

#include <vector>

struct Mesh;
struct MeshPrimitive;
struct Material;
struct TextureData;
struct Accessor;
struct BufferView;
struct BufferData;

struct Mesh {
  std::vector<MeshPrimitive> primitives{};
  std::vector<float> weights{};
};

struct MeshPrimitive {
  using Mode = fx::gltf::Primitive::Mode;
  using Attributes = std::unordered_map<std::string, Accessor*>;
  using AttributeMap = std::unordered_map<std::string, GLuint>;

  Mode mode = Mode::Triangles;
  Attributes attributes = {};
  Accessor* indices = nullptr;

  // std::vector<Attributes> morphTargets{};

  GLuint vaoId = 0;

  bool isLoaded() const;
  void loadToGpu(const AttributeMap& attributeMap, bool reload = false);
};

struct Material {
  glm::vec4 baseColorFactor = glm::vec4(0.5);
  TextureData* baseColorTexture = nullptr;

  TextureData* normalMap = nullptr;

  bool isLoaded() const;
  void loadToGpu(bool reload = false);
};

struct TextureData {
  std::vector<uint8_t> data = {};
  int width = 0;
  int height = 0;

  fx::gltf::Sampler sampler;

  GLuint texId = 0;

  bool isLoaded() const;
  void loadToGpu(bool reload = false);
};

struct Accessor {
  using Type = fx::gltf::Accessor::Type;
  using ComponentType = fx::gltf::Accessor::ComponentType;

  // struct Sparse {};

  BufferView* bufferView = nullptr;

  uint32_t byteOffset = 0;
  uint32_t count = 0;

  Type type = Type::None;
  ComponentType componentType = ComponentType::None;
  bool normalized = false;

  // Sparse sparse{};

  std::vector<float> max = {};
  std::vector<float> min = {};
};

struct BufferView {
  using TargetType = fx::gltf::BufferView::TargetType;

  BufferData* buffer = nullptr;
  uint32_t byteOffset = 0;
  uint32_t byteLength = 0;
  uint32_t byteStride = 0;

  // unused
  TargetType target = TargetType::None;

  GLuint vboId = 0;

  bool isLoaded() const;
  void loadToGpu(bool reload = false);
};

struct BufferData {
  std::vector<uint8_t> data = {};
};

BufferView* createBufferView(const std::vector<float>& bufferData);

#endif // !PRIMITIVES_H
