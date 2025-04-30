#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
uniform mat4 projection;
uniform mat4 view;
out vec2 TexCoords;
void main() {
    vec4 pos = projection * view * vec4(aPos, 1.0);
		TexCoords = aTexCoords;
    gl_Position = pos.xyww; // Set depth to far plane
}
