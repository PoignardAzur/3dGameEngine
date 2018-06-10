#version 400

// c_ : color
// tc_ : texture coordinates
// tsc_ : tangentSpace coordinates

uniform mat4 modelView;

uniform vec4 c_materialColor;
uniform sampler2D textureId;
uniform sampler2D normalMapId;

in vec3 tsc_lightDir;
in vec3 tsc_viewpointDir;
in vec2 tc_texture;

out vec4 c_fragColor;

void main()
{
  vec3 tsc_normal = texture(normalMapId, tc_texture).xyz;

  float diffuseIntensity = max(dot(tsc_lightDir, tsc_normal), 0);

  vec4 c_textureColor = texture(textureId, tc_texture);
  c_fragColor = (
    vec4(vec3(diffuseIntensity), 1)
    * c_materialColor
    * c_textureColor
  );
}
