#version 400

in vec3 POSITION;

out vec3 c_color;

void main(void)
{
  gl_Position = vec4(POSITION, 1.0);
  c_color = vec3(1);
}
