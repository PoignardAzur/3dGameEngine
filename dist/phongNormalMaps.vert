#version 400

// oc_ : object coordinates
// cc_ : camera coordinates
// tsc_ : tangentSpace coordinates
// tc_ : texture coordinates

uniform mat4 mvp;
uniform mat4 modelView;
uniform mat3 normalMatrix;

uniform vec3 cc_lightPos;

in vec3 POSITION;
in vec3 NORMAL;
in vec4 TANGENT;
in vec2 TEXCOORD_0;

out vec3 tsc_lightDir;
out vec3 tsc_viewpointDir;
out vec2 tc_texture;

void main(void)
{
  vec3 oc_vertexTangent = TANGENT.xyz;
  vec3 oc_vertexBitangent = cross(NORMAL, TANGENT.xyz) * TANGENT.w;
  vec3 oc_vertexNormal = NORMAL;
  tc_texture = TEXCOORD_0;

  vec3 cc_tangent = mat3(modelView) * oc_vertexTangent;
  vec3 cc_bitangent = mat3(modelView) * oc_vertexBitangent;
  vec3 cc_normal = normalMatrix * oc_vertexNormal;

  mat3 tbn = mat3(cc_tangent, cc_bitangent, cc_normal);
  // tbn is orthogonal, which means transpose(tbn) == inverse(tbn)
  mat3 cc_to_tsc = transpose(tbn);

  vec3 cc_vertexPos = mat3(modelView) * POSITION;
  tsc_lightDir = cc_to_tsc * (cc_lightPos - cc_vertexPos);
  tsc_viewpointDir = cc_to_tsc * (-cc_vertexPos);

  gl_Position = mvp * vec4(POSITION, 1.0);
}
