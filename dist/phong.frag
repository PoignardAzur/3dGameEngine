#version 400

// c_ : color
// cc_ : camera coordinates

uniform mat4 modelView;
uniform vec4 cc_lightPos;

in vec3 cc_pos;
in vec3 cc_normal;

out vec4 c_fragColor;

void main()
{
  vec3 cc_lightdir = normalize(cc_lightPos.xyz - cc_pos);

  vec3 cc_reflectedLight = normalize(cc_lightdir - cc_pos);
  float specularShine = max(dot(cc_reflectedLight, cc_normal), 0.0);

  float diffuseIntensity = max(dot(cc_lightdir, cc_normal), 0);

  c_fragColor = vec4(vec3(diffuseIntensity), 1);
}
