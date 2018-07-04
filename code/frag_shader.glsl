#version 330

in vec4 Color; 
in vec2 TexCoord;

out vec4 FragColor;

void main()
{
    FragColor = Color;
}