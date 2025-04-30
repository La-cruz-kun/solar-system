#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D equirectangularMap;


void main() {
    FragColor = texture(equirectangularMap, TexCoords) * vec4(0.5, 0.5, 0.5, 1);
}
