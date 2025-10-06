#version 450

layout(location = 0) in vec3 fragNorm;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragModelPos;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

const float ambientStrength = 0.1f;
const vec3 lightColor = vec3(1.0f);
const vec3 lightPos = vec3(50, 20, 50);
const vec3 camPos = vec3(50, 50, -50);
const float specularStrength = 0.5;

void main()
{
    // diffuse
    vec3 norm = normalize(fragNorm);
    vec3 lightDir = normalize(lightPos - fragModelPos);  
    float diff = max(dot(norm, lightDir), 0.0);

    // specular
    vec3 viewDir = normalize(camPos - fragModelPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    float lightStrength = specularStrength * spec + ambientStrength + diff;
    outColor = vec4(lightColor * lightStrength, 1.0f) * texture(texSampler, fragUV); 
}
