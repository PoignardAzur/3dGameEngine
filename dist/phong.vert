#version 400

// oc_ : object coordinates
// cc_ : camera coordinates

uniform mat4 mvp;
uniform mat4 modelView;
uniform mat3 normalMatrix;

uniform mat4 oc_jointMatrices[16];

in vec3 POSITION;
in vec3 NORMAL;
in vec2 TEXCOORD_0;
in vec4 JOINTS_0;
in vec4 WEIGHTS_0;

out vec3 cc_pos;
out vec3 cc_normal;
out vec2 tc_texture;

void main(void)
{
  mat4 skinMatrix = (
    WEIGHTS_0.x * oc_jointMatrices[int(JOINTS_0.x)]
    + WEIGHTS_0.y * oc_jointMatrices[int(JOINTS_0.y)]
    + WEIGHTS_0.z * oc_jointMatrices[int(JOINTS_0.z)]
    + WEIGHTS_0.w * oc_jointMatrices[int(JOINTS_0.w)]
  );

  gl_Position = mvp * skinMatrix * vec4(POSITION, 1.0);

  cc_pos = (modelView * vec4(POSITION, 1.0)).xyz;
  cc_normal = normalize(normalMatrix * NORMAL);
  tc_texture = TEXCOORD_0;
}
