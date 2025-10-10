#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inUV;

layout(set = 0, binding = 0) uniform FrameUBO {
    mat4 view;
    mat4 proj;
} u_frame;

layout(set = 2, binding = 0) uniform ObjectUBO {
    mat4 model;
} u_object;

layout(location = 0) out vec3 fragNorm;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragModelPos;

void main()
{
    vec4 modelPos = u_object.model * vec4(inPosition, 1.0);
    fragModelPos = vec3(modelPos);
    gl_Position = u_frame.proj * u_frame.view * modelPos;
    fragNorm = mat3(transpose(inverse(u_object.model))) * inNorm;
    fragUV = inUV;
}