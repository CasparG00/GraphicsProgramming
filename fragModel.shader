#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normals;
in vec4 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_ao1;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void clip(float d, float edge) {
    if (d <= edge) discard;
}

void main()
{
    vec3 lightDir = normalize(vec3(lightDirection));
    
    vec4 diffuse = texture(texture_diffuse1, TexCoords);
    clip(diffuse.a, 0.5);
    vec4 specTex = texture(texture_specular1, TexCoords);

    float light = max(abs(dot(-lightDir, Normals)), 0.0);

    vec3 viewDir = normalize(cameraPosition - FragPos.rgb);
    vec3 refl = reflect(lightDir, Normals);

    float ambientOcclusion = texture(texture_ao1, TexCoords).r;
    
    float roughness = texture(texture_roughness1, TexCoords).r;
    float spec = pow(max(dot(viewDir, refl), 0.0), lerp(1, 128, roughness));
    vec3 specular = spec * specTex.rgb;

    vec4 result = diffuse * light + vec4(specular, 0);
    result.a = light;

    FragColor = result;
}