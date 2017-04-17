#version 450

layout(location = 0) in vec4 texcoord;
layout(location = 0) out vec4 color;

void main() {
    color = vec4(texcoord.xyz, 1.0);
}
