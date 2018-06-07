#version 400

in vec3 c_color;

out vec4 c_fragColor;

void main()
{
  c_fragColor = vec4(c_color, 1.0);
}
