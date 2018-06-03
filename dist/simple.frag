#version 400

in vec3 c_color;

out vec4 FragColors;

void main()
{
  FragColors = vec4(c_color, 1.0);
}
