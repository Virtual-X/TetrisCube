#version 400

uniform sampler2D backBuffer;
uniform sampler3D volume;

in vec4 front;
in vec4 backPos;
in vec2 tex;

out vec4 result;

void main()
{
    float delta = 0.01;

    vec2 backCoord = ((backPos.xy / backPos.w) + 1) / 2;
    vec3 end = texture2D(backBuffer, backCoord).xyz;
    vec3 start = front.xyz;

    vec3 dir = end - start;
    float len = length(dir);
    if (len == 0)
        discard;
    delta = max(delta, len / 100);
    vec3 step = normalize(dir) * delta;

    float pos = 0;
    vec4 color = vec4(0, 0, 0, 0);
    float alpha = 0;
    float c = 0;
    while (alpha < 0.99f && pos < len) {
        vec4 col = texture(volume, start.xyz / 2 + 0.5);
        float a = col.a;
//        if (a > alpha) {
//            color = col;
//            alpha = a;
//        }
        if (a > 0.3) {
            a = a - 0.3;
            color += (1 - alpha) * a * col;
            alpha += a / 2;
        }

        pos += delta;
        start += step;
        c++;
    }

    //color += vec4(len / 3, 0, 0, 1) - color;
    //vec4 tex = clamp(texture(volume, vec3(0.5, 0.5, 0.5).xy), 0, 1);
    result = clamp(vec4(color.xy, len / 20, 1), 0, 1); // * 0 + vec4(tex.rg, 0.2, 1);
}
