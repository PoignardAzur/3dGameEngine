
#include <cassert>
#include "Primitives.hpp"

// TODO - Integrate to gltf lib
static inline uint32_t _getComponentCount(Accessor::Type accessorType) {
  switch (accessorType) {
    case Accessor::Type::None: {
      break;
    }
    case Accessor::Type::Scalar:
    case Accessor::Type::Vec2:
    case Accessor::Type::Vec3:
    case Accessor::Type::Vec4: {
      return (int)accessorType - (int)Accessor::Type::Scalar + 1;
    }
    case Accessor::Type::Mat2:
    case Accessor::Type::Mat3:
    case Accessor::Type::Mat4: {
      int count = (int)accessorType - (int)Accessor::Type::Mat2 + 2;
      return count * count;
    }
  }
  assert(false);
}

static inline uint32_t _getComponentSize(Accessor::ComponentType componentType) {
  switch (componentType) {
    case Accessor::ComponentType::None: {
      break;
    }
    case Accessor::ComponentType::Byte: {
      return 1;
    }
    case Accessor::ComponentType::UnsignedByte: {
      return 1;
    }
    case Accessor::ComponentType::Short: {
      return 2;
    }
    case Accessor::ComponentType::UnsignedShort: {
      return 2;
    }
    case Accessor::ComponentType::UnsignedInt: {
      return 4;
    }
    case Accessor::ComponentType::Float: {
      return 4;
    }
  }
  assert(false);
}

bool MeshPrimitive::isLoaded() const {
  for (const auto& attribute: this->attributes) {
    const Accessor* accessor = attribute.second;
    if (!accessor->bufferView->isLoaded()) {
      return false;
    }
  }

  return (this->vaoId != 0);
}

void MeshPrimitive::loadToGpu(const AttributeMap& attributeMap, bool reload) {
  if (this->vaoId == 0) {
    glGenVertexArrays(1, &this->vaoId);
    // TODO - error handling
  }
  else if (!reload) {
    return;
  }

  // TODO - reload bufferViews

  for (const auto& attribute: this->attributes) {
    const Accessor* accessor = attribute.second;
    accessor->bufferView->loadToGpu(false);
  }

  glBindVertexArray(this->vaoId);

  for (const auto& attribute: this->attributes) {
    const std::string& name = attribute.first;
    const Accessor* accessor = attribute.second;

    auto it = attributeMap.find(name);
    if (it != attributeMap.end()) {
      GLuint attributeIndex = it->second;
      int componentCount = _getComponentCount(accessor->type);

      glBindBuffer(GL_ARRAY_BUFFER, accessor->bufferView->vboId);
      glEnableVertexAttribArray(attributeIndex);
      glVertexAttribPointer(
        attributeIndex,
        componentCount,
        (GLenum)accessor->componentType,
        accessor->normalized,
        accessor->bufferView->byteStride,
        reinterpret_cast<GLvoid*>(accessor->byteOffset)
      );
    }
  }

  if (this->indices) {
    this->indices->bufferView->loadToGpu(reload);
  }
}


bool Material::isLoaded() const {
  TextureData* textures[]  = {
    this->baseColorTexture,
    this->normalMap
  };
  for (TextureData* texture: textures) {
    if (!texture->isLoaded()) {
      return false;
    }
  }
  return true;
}

void Material::loadToGpu(bool reload) {
  TextureData* textures[]  = {
    this->baseColorTexture,
    this->normalMap
  };
  for (TextureData* texture: textures) {
    texture->loadToGpu(reload);
  }
}


bool TextureData::isLoaded() const {
  return (this->texId != 0);
}

void TextureData::loadToGpu(bool reload) {
  if (this->isLoaded()) {
    if (reload) {
      glDeleteTextures(1, &this->texId);
      this->texId = 0;
    }
    else {
      return;
    }
  }

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &this->texId);
  glBindTexture(GL_TEXTURE_2D, this->texId);

  if (this->sampler.magFilter != fx::gltf::Sampler::MagFilter::None) {
    glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)this->sampler.magFilter
    );
  }
  if (this->sampler.minFilter != fx::gltf::Sampler::MinFilter::None) {
    glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)this->sampler.minFilter
    );
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)this->sampler.wrapS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)this->sampler.wrapT);

  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height,
    0,
    GL_RGBA, GL_UNSIGNED_BYTE, this->data.data()
  );
  glGenerateMipmap(GL_TEXTURE_2D);
}


inline uint32_t Accessor::getStride() const {
  uint32_t stride = this->bufferView->byteStride;
  return stride != 0
    ? stride
    : _getComponentCount(this->type) * _getComponentSize(this->componentType)
  ;
}

float Accessor::getComponent(uint32_t element, uint32_t component) const {
  assert(component < _getComponentCount(this->type));

  const void* compData = (
    this->bufferView->buffer->data.data()
    + this->byteOffset + this->bufferView->byteOffset
    + element * this->getStride()
    + component * _getComponentSize(this->componentType)
  );

  switch (this->componentType) {
    case Accessor::ComponentType::None: {
      break;
    }
    case ComponentType::Byte: {
      float f = *(int8_t*)compData;
      return this->normalized ? std::max(f / 127.0, -1.0) : f;
    }
    case ComponentType::UnsignedByte: {
      float f = *(uint8_t*)compData;
      return this->normalized ? f / 255.0 : f;
    }
    case ComponentType::Short: {
      float f = *(int16_t*)compData;
      return this->normalized ? std::max(f / 32767.0, -1.0) : f;
    }
    case ComponentType::UnsignedShort: {
      float f = *(uint16_t*)compData;
      return this->normalized ? f / 65535.0 : f;
    }
    case ComponentType::UnsignedInt: {
      float f = *(uint16_t*)compData;
      return this->normalized ? f / 4294967295.0 : f;
    }
    case ComponentType::Float: {
      return *(float*)compData;
    }
  }
  assert(false);
}


bool BufferView::isLoaded() const {
  return (this->vboId != 0);
}

void BufferView::loadToGpu(bool reload) {
  if (this->vboId == 0) {
    glGenBuffers(1, &this->vboId);
    // TODO - error handling
  }
  else if (!reload) {
    return;
  }

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, this->vboId);

  assert(this->byteOffset + this->byteLength <= this->buffer->data.size());
  glBufferData(
    GL_ARRAY_BUFFER,
    this->byteLength,
    this->buffer->data.data() + this->byteOffset,
    GL_STATIC_DRAW
  );
}

BufferView* createBufferView(const std::vector<float>& bufferData) {
  // TODO - handle endianness

  const uint8_t* rawPtr = reinterpret_cast<const uint8_t*>(bufferData.data());
  std::vector<uint8_t> rawData(
    rawPtr, rawPtr + bufferData.size() * sizeof(float)
  );

  return new BufferView {
    new BufferData { rawData },
    0, (uint32_t)rawData.size(), 0
  };
}
