#version 400

uniform mat4 mvp;

in vec3 POSITION;

out vec3 c_color;

void main(void)
{
  gl_Position = mvp * vec4(POSITION, 1.0);
  c_color = vec3(1);
}
