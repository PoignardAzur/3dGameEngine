
#include <cassert>
#include "draw.hpp"

void draw(ShaderProgram& shaderProgram, MeshPrimitive& meshPrimitive) {
  assert(meshPrimitive.isLoaded());
  assert(meshPrimitive.attributes.count("POSITION") > 0);

  shaderProgram.use();
  glBindVertexArray(meshPrimitive.vaoId);

  if (meshPrimitive.indices) {
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshPrimitive.indices->bufferView->vboId);
    glDrawElements(
      (GLenum)meshPrimitive.mode,
      meshPrimitive.indices->count,
      (GLenum)meshPrimitive.indices->type,
      reinterpret_cast<GLvoid*>(meshPrimitive.indices->byteOffset)
    );
  }
  else {
    glDrawArrays(
      (GLenum)meshPrimitive.mode,
      0,
      meshPrimitive.attributes["POSITION"]->count
    );
  }

  glBindVertexArray(0);
  shaderProgram.disable();
}
