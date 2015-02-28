#version 400

uniform mat4 mvp;

in vec3 pos;

out vec4 front;
out vec4 backPos;

void main ()
{
    front = vec4(pos, 1.0);
    backPos = mvp * front;
    gl_Position = mvp * front;
}
