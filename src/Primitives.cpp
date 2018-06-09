
#include <cassert>
#include "Primitives.hpp"

// TODO - Integrate to gltf lib
static int _getComponentCount(Accessor::Type accessorType) {
  switch (accessorType) {
    case Accessor::Type::None: {
      assert(false);
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
    this->baseColorTexture
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
    this->baseColorTexture
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
