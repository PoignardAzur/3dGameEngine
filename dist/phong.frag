#version 400

// c_ : color
// cc_ : camera coordinates

uniform mat4 modelView;
uniform vec3 cc_lightPos;

uniform vec4 c_materialColor;
uniform sampler2D textureId;

in vec3 cc_pos;
in vec3 cc_normal;
in vec2 tc_texture;

out vec4 c_fragColor;

void main()
{
  vec3 cc_lightdir = normalize(cc_lightPos - cc_pos);

  vec3 cc_reflectedLight = normalize(cc_lightdir - cc_pos);
  float specularShine = max(dot(cc_reflectedLight, cc_normal), 0.0);

  float diffuseIntensity = max(dot(cc_lightdir, cc_normal), 0);

  vec4 c_textureColor = texture(textureId, tc_texture);
  c_fragColor = (
    vec4(vec3(diffuseIntensity), 1)
    * c_materialColor
    * c_textureColor
  );
}
