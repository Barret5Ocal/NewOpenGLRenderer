#version 130

in vec2 FSTextureCoordinates;
in vec4 FSColor;

out vec4 Color;

uniform sampler2D FontTexture;

void main()
{
    float C = texture(FontTexture, FSTextureCoordinates).r;
    Color =  C * FSColor;
}
