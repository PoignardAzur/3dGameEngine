#version 400

// oc_ : global coordinates
// cc_ : camera coordinates

uniform mat4 mvp;
uniform mat4 modelView;
uniform mat3 normalMatrix;

in vec3 POSITION;
in vec3 NORMAL;

out vec3 cc_pos;
out vec3 cc_normal;

void main(void)
{
  gl_Position = mvp * vec4(POSITION, 1.0);

  cc_pos = (modelView * vec4(POSITION, 1.0)).xyz;
  cc_normal = normalize(normalMatrix * NORMAL);
}
