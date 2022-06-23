#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

vec3 palette[4] = vec3[4](
    vec3(15, 56, 15),
    vec3(48, 98, 48),
    vec3(139, 172, 15),
    vec3(155, 188, 15)
);

void main()
{
    vec3 original = texture(screenTexture, TexCoords).rgb;

    vec3 col = vec3(0, 0, 0);
    float dist = 10000000.0;

    for (int i = 0; i < 4; i++) 
    {
        vec3 c = palette[i] / 255;
        float d = distance(original, c);

        if (d < dist) 
        {
            dist = d;
            col = c;
        }
    }

    FragColor = vec4(col, 1.0f);
}