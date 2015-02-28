#version 400

in vec3 pos;

uniform mat4 mvp;

out vec4 back;

void main ()
{
    back = vec4(pos, 1.0);
    gl_Position = mvp * back;
}
