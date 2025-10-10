#version 450

layout(location = 0) in vec3 fragNorm;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragModelPos;

layout(set = 1, binding = 0) uniform sampler2D albedo;

layout(set = 0, binding = 1) uniform FrameLight
{
    vec4 light; // xyz position, w attenuation
} u_light;

layout(location = 0) out vec4 outColor;

const float ambientStrength = 0.1f;
const vec3 lightColor = vec3(1.0f);
const vec3 camPos = vec3(50, 50, -50);
const float specularStrength = 0.5;

void main()
{
    vec3 lightPos = u_light.light.xyz;
    float attenuation = u_light.light.w;

    // diffuse
    vec3 norm = normalize(fragNorm);
    vec3 lightDir = normalize(lightPos - fragModelPos);  
    float diff = max(dot(norm, lightDir), 0.0);

    // specular
    vec3 viewDir = normalize(camPos - fragModelPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    float lightStrength = specularStrength * spec + ambientStrength + diff;
    outColor = vec4(attenuation * lightColor * lightStrength, 1.0f) * texture(albedo, fragUV); 
}
