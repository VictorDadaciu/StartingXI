#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inUV;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec3 fragNorm;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragModelPos;

void main()
{
    vec4 modelPos = ubo.model * vec4(inPosition, 1.0);
    fragModelPos = vec3(modelPos);
    gl_Position = ubo.proj * ubo.view * modelPos;
    fragNorm = mat3(transpose(inverse(ubo.model))) * inNorm;
    fragUV = inUV;
}